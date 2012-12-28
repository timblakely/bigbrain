"""Handles database resource management."""

import itertools
import math
import re
import time

from bigbrain.cluster.cluster_manager import InstanceNotFound
import bson
import tornado.ioloop as ioloop


def floatrange(start, end, step=1.0):
  v = start
  while (step > 0 and v < end) or (step < 0 and v > end):
    yield v
    v += step
    v = round(v, 15)
  raise StopIteration


class ResourceController(object):
  """Controls all BigBrain front-end resources."""

  def __init__(self, cluster, rpc, db):
    self._cluster = cluster
    self._rpc = rpc
    self._db = db
    # List group names
    self._group_names = [group for group in db.groups.find({}, {'name': 1})]
    self._user_names = [user for user in db.users.find({}, {'name': 1})]

  def AddExperiment(self, username, description):
    self._db.resources.save({
        'type': 'experiment',
        'name': description['name'],
        'description': description['description'],
        'owner': username,
        'editors': [username],
        'shared': [],
        'public': False,
        'status': 'Stopped',
        'compilation': {
            'success': False,
            'errors': [],
            'warnings': []
        },
        'code': '',
        'running': False,
        'parameters': []})

  def ListExperiments(self, username, access_type='all'):
    """List experiments the user has access to."""
    query = {'type': 'experiment'}
    if access_type == 'all':
      query['$or'] = [{'owner': username}, {'shared': username}]
    elif access_type == 'private':
      query['owner'] = username
    elif access_type == 'shared':
      query['shared'] = username
    else:
      return []
    cursor = self._db.resources.find(query)
    return [expt for expt in cursor]

  def StopExperiment(self, mongodb_id):
    """Terminates experiment and upates DB."""
    try:
      obj_id = bson.ObjectId(mongodb_id)
    except bson.objectid.InvalidId:
      return False

    # First, purge previous experiments
    cursor = self._db.resources.find({
        'type': 'instance',
        'expt_id': obj_id
    })

    for instance in cursor:
      try:
        self._cluster.TearDownInstance(instance['name'])
      except InstanceNotFound:
        pass

    self._db.resources.update(
        {
            'type': 'experiment',
            '_id': obj_id,
        },
        {
            '$set': {'running': False}
        })

  def ExecuteExperiment(self, mongodb_id):
    """Tear down old experiment and start all new instances."""
    try:
      obj_id = bson.ObjectId(mongodb_id)
    except bson.objectid.InvalidId:
      return False

    # First, purge previous experiments
    cursor = self._db.resources.find({
        'type': 'instance',
        'expt_id': obj_id
    })

    for instance in cursor:
      try:
        self._cluster.TearDownInstance(instance['name'])
      except InstanceNotFound:
        pass

    self._db.resources.remove({
        'type': 'instance',
        'expt_id': obj_id
    })

    cursor = self._db.resources.find({
        'type': 'experiment',
        '_id': obj_id
    })
    if cursor.count() != 1:
      return False

    expt = cursor[0]

    def CreateExperimentInstance(parameters):
      """Space out instance creation so we don't hang during gAPI."""

      code = expt['code']
      for param, value in parameters.iteritems():
        code = re.sub(r'\{\_\{\s*' + param + r'\s*\}\_\}', str(value), code)

      instance = self._cluster.StartNewInstance()
      if not instance:
        return False

      self.AddInstance(instance, expt['name'], expt['_id'], parameters,
                       'Starting')

      def InstanceRunning():
        # Instance is up and ready

        self.UpdateInstanceStatus(instance, 'Running')

        def DoneGenerating(response):  # pylint: disable-msg=W0613
          self._rpc.CallRPC(instance, 'RunUntil', {'dt': 0.25, 't': 4000.0})

        self._rpc.CallRPC(instance, 'GenerateNetwork', {'description': code},
                          DoneGenerating)

      def WaitUntilRunning():
        try:
          if (not self._cluster.IsRunning(instance) and
              not self._cluster.IsFailed(instance)):
            ioloop.IOLoop.instance().add_timeout(
                time.time() + 5, WaitUntilRunning)
            return
          if self._cluster.IsFailed(instance):
            # Instance failed before it started. Restart?
            # TODO(blakely): notify users
            print 'DEBUG: FAILED BEFORE STARTING'
            return
          self.UpdateInstanceStatus(instance, 'Ready')
          ioloop.IOLoop.instance().add_timeout(
              time.time() + 30, InstanceRunning)
        except InstanceNotFound:
          ioloop.IOLoop.instance().add_timeout(
              time.time() + 5, WaitUntilRunning)

      ioloop.IOLoop.instance().add_callback(WaitUntilRunning)

    parameter_sets = []
    for param in expt['parameters']:
      if not param['enabled']:
        continue
      parameter_sets.append(
          list(floatrange(param['min'], param['max'], param['step'])))

    param_names = [param['name'] for param in expt['parameters']
                   if param['enabled']]
    param_values = list(itertools.product(*parameter_sets))

    def MakeCallback(a, b):
      def Callback():
        params = dict(zip(a, b))
        CreateExperimentInstance(params)
      return Callback

    delay = 0
    delay_step = 0.1

    for param_set in param_values:
      delay += delay_step
      ioloop.IOLoop.instance().add_timeout(
          time.time() + delay, MakeCallback(param_names, param_set))

    self._db.resources.update(
        {
            'type': 'experiment',
            '_id': obj_id,
        },
        {
            '$set': {'running': True}
        })

  def GetExperiment(self, username, mongodb_id):
    """Returns the resource if the user has access to it."""
    try:
      obj_id = bson.ObjectId(mongodb_id)
    except bson.objectid.InvalidId:
      return None
    cursor = self._db.resources.find(
        {
            'type': 'experiment',
            '_id': obj_id,
            '$or': [
                {
                    'owner': username
                },
                {
                    'shared': username
                }]
        })
    if cursor.count() == 1:
      return cursor[0]
    return None

  def FindExperimentInstance(self, mongodb_id, params):
    """See if the instance is a known one in the database."""
    try:
      obj_id = bson.ObjectId(mongodb_id)
    except bson.objectid.InvalidId:
      return None
    query = {
        'type': 'instance',
        'expt_id': obj_id
    }

    for p in params:
      # TODO(blakely): This needs work
      try:
        query['parameters.' + p[0]] = float(p[1])
      except ValueError:
        query['parameters.' + p[0]] = p[1]

    cursor = self._db.resources.find(query)
    if cursor.count() == 1:
      return cursor[0]
    return None

  def GetExperimentInstances(self, mongodb_id):
    """Returns the resource if the user has access to it."""
    try:
      obj_id = bson.ObjectId(mongodb_id)
    except bson.objectid.InvalidId:
      return None
    cursor = self._db.resources.find(
        {
            'type': 'instance',
            'expt_id': obj_id
        })
    return list(cursor)

  def UserExists(self, username):
    cursor = self._db.users.find({'user': username}, {'user': 1})
    return cursor.count() == 1

  def CanEditExperiment(self, username, mongodb_id):
    """Test if a user can edit an experiment."""
    try:
      obj_id = bson.ObjectId(mongodb_id)
    except bson.objectid.InvalidId:
      return False
    cursor = self._db.resources.find(
        {
            'type': 'experiment',
            '_id': obj_id,
            '$or': [
                {
                    'owner': username
                },
                {
                    'editors': username
                }]
        })
    if cursor.count() == 1:
      return True
    return False

  def ShareExperimentWithUser(self, mongodb_id, with_username):
    try:
      obj_id = bson.ObjectId(mongodb_id)
    except bson.objectid.InvalidId:
      return False
    self._db.resources.update(
        {
            'type': 'experiment',
            '_id': obj_id},
        {
            '$addToSet': {'shared': with_username}
        })

  def SaveExperimentCode(self, user, mongodb_id, description):
    try:
      obj_id = bson.ObjectId(mongodb_id)
    except bson.objectid.InvalidId:
      return False
    self._db.resources.update(
        {
            'type': 'experiment',
            '_id': obj_id,
            '$or': [{'owner': user}, {'editors': user}]
        },
        {
            '$set': {'code': description, 'compilation.success': False}
        })

  def ExperimentIsCompiled(self, mongodb_id):
    """Test to see if the experiment has been compiled."""
    try:
      obj_id = bson.ObjectId(mongodb_id)
    except bson.objectid.InvalidId:
      return False

    cursor = self._db.resources.find(
        {
            'type': 'experiment',
            '_id': obj_id
        },
        {
            'compilation.success': 1
        })
    return cursor.count() != 0

  def ExperimentIsRunning(self, mongodb_id):
    """Is the experiment currently running."""
    try:
      obj_id = bson.ObjectId(mongodb_id)
    except bson.objectid.InvalidId:
      return False

    cursor = self._db.resources.find(
        {
            'type': 'experiment',
            '_id': obj_id
        },
        {
            'status': 1
        })
    return cursor[0]['status'] == 'Running'

  def NumInstancesInExperiment(self, mongodb_id):
    """List the number of instances that the experiment could spawn."""
    try:
      obj_id = bson.ObjectId(mongodb_id)
    except bson.objectid.InvalidId:
      return None
    cursor = self._db.resources.find({
        'type': 'experiment',
        '_id': obj_id
    })
    if cursor.count() == 0:
      return None

    expt = cursor[0]
    num_instances = 0
    for param in expt['parameters']:
      num_instances += int(
          math.floor((param['max'] - param['min']) / param['step']) + 1)
    return num_instances

  def CompileExperiment(self, user, mongodb_id):
    """Compile the code and params, ensuring the experiment can be executed."""
    compilation = {
        'success': False,
        'errors': [],
        'warnings': []
    }
    try:
      obj_id = bson.ObjectId(mongodb_id)
    except bson.objectid.InvalidId:
      compilation['errors'].append('Invalid mongodb_id')
      return compilation
    cursor = self._db.resources.find({
        'type': 'experiment',
        '_id': obj_id
    })
    if cursor.count() == 0:
      compilation['errors'].append('Couldn\'t find experiment')
      return compilation

    expt = cursor[0]

    if expt['compilation']['success']:
      return expt['compilation']

    if user not in expt['owner'] and user not in expt['editors']:
      compilation['errors'].append(
          'You don\'t have access to compile this experiment')
      return compilation

    # Find all parameters defined in text file
    pattern = re.compile(r'\{\_\{\s*(.*?)\s*\}\_\}')
    params_in_code = [(m.group(1), m.start()) for m in pattern.finditer(
        expt['code'])]

    # Test if the variables defined in code exist
    for p in params_in_code:
      expt_param = [v for v in expt['parameters'] if v['name'] == p[0]]
      if not expt_param:
        compilation['errors'].append('"%s" is not a known parameter' % p[0])
        continue
      expt_param = expt_param[0]
      if not expt_param:
        compilation['errors'].append('"%s" not defined '
                                     'but used in code' % p[0])
      elif not expt_param['enabled']:
        compilation['errors'].append('"%s" used, but not enabled' % p[0])

    # Warn if the variables defined are not used in code
    for expt_param in expt['parameters']:
      if expt_param['name'] not in [v[0] for v in params_in_code]:
        if expt_param['enabled']:
          compilation['warnings'].append(
              '"%s" enabled but not used in code' % expt_param['name'])

    if not compilation['errors']:
      compilation['success'] = True

    self._db.resources.update(
        {
            'type': 'experiment',
            '_id': obj_id,
            '$or': [{'owner': user}, {'editors': user}]
        },
        {
            '$set': {'compilation': compilation}
        })
    return compilation

  def SetParameters(self, username, mongodb_id, parameters):
    """Set parameters for experiment."""
    try:
      obj_id = bson.ObjectId(mongodb_id)
    except bson.objectid.InvalidId:
      return False
    self._db.resources.update(
        {
            'type': 'experiment',
            '_id': obj_id,
            '$or': [{'owner': username}, {'editors': username}]
        },
        {
            '$set': {'parameters': parameters, 'compilation.success': False}
        })
    return True

  def AddParameter(self, username, mongodb_id, parameter):
    """Set parameters for experiment."""
    try:
      obj_id = bson.ObjectId(mongodb_id)
    except bson.objectid.InvalidId:
      return False
    self._db.resources.update(
        {
            'type': 'experiment',
            '_id': obj_id,
            '$or': [{'owner': username}, {'editors': username}]
        },
        {
            '$push': {'parameters': parameter},
            '$set': {'compilation.success': False}
        })
    return True

  def DeleteParameter(self, username, mongodb_id, parameter):
    """Set parameters for experiment."""
    try:
      obj_id = bson.ObjectId(mongodb_id)
    except bson.objectid.InvalidId:
      return False
    self._db.resources.update(
        {
            'type': 'experiment',
            '_id': obj_id,
            '$or': [{'owner': username}, {'editors': username}]
        },
        {
            '$pull': {'parameters': parameter},
            '$set': {'compilation.success': False}
        })
    return True

  def UpdateParameter(self, username, mongodb_id, parameter):
    """Set parameters for experiment."""
    try:
      obj_id = bson.ObjectId(mongodb_id)
    except bson.objectid.InvalidId:
      return False
    query = {
        'type': 'experiment',
        '_id': obj_id,
        '$or': [{'owner': username}, {'editors': username}],
        'parameters.name': parameter['name']
    }
    todo = {
        '$set': {
            'parameters.$': parameter,
            'compilation.success': False
        }
    }
    self._db.resources.update(query, todo)
    return True

  def GetParameters(self, username, mongodb_id):
    """Get the parameters associated with experiment."""
    try:
      obj_id = bson.ObjectId(mongodb_id)
    except bson.objectid.InvalidId:
      return []
    cursor = self._db.resources.find(
        {
            'type': 'experiment',
            '_id': obj_id,
            '$or': [{'owner': username}, {'shared': username}]
        },
        {
            'parameters': 1
        })
    if cursor.count():
      return cursor[0]
    return []

  def CanAddInstance(self, user, group='private'):
    cursor = self._db.users.find({'user': user}, {'groups': 1, 'resources': 1})
    if not cursor.count():
      return False
    # User is authenticated for this group
    return (group in cursor[0]['groups']) or (group == 'private')

  def AddInstance(self, instance, experiment_name, mongodb_id, parameters,
                  status):
    self._db.resources.save(
        {
            'type': 'instance',
            'name': instance,
            'expt_name': experiment_name,
            'expt_id': mongodb_id,
            'parameters': parameters,
            'status': status
        })

  def UpdateInstanceStatus(self, instance, status):
    self._db.resources.update(
        {
            'type': 'instance',
            'name': instance
        },
        {
            '$set': {'status': status}
        })

  def RemoveInstance(self, instance):
    self.RemoveInstances([instance])

  def RemoveInstances(self, instances):
    for instance in instances:
      self._db.remove(
          {
              'type': 'instance',
              'name': instance
          })
