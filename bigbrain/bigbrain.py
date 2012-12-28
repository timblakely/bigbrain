# Copyright 2012 Google Inc. All Rights Reserved.

"""Rewrite of BigBrains front-end."""

import gzip
import inspect
import json
import logging
import optparse
import socket
import StringIO
import sys
import time

import msgpack
import pymongo
from tornado import ioloop
from tornado import web
import tornado.escape

from bigbrain.cluster import google_compute_cluster_manager as gccm
from bigbrain.cluster import localhost_cluster_manager as lcm
import bigbrain.resourcecontroller
import bigbrain.rpc
from bigbrain.streams import viewstream


class BaseHandler(web.RequestHandler):
  """Main page handler."""

  def initialize(self, db, cluster, rpc, rc):  # pylint: disable-msg=C6409
    """Handles the basic drawing of the main webpage."""
    self._db = db
    self._cluster = cluster
    self._rpc = rpc
    self._rc = rc
    self._user = self.get_current_user()

  def get_login_url(self):  # pylint: disable-msg=C6409
    return u'/login'

  def get_current_user(self):  # pylint: disable-msg=C6409
    user_json = self.get_secure_cookie('user')
    if user_json:
      return tornado.escape.json_decode(user_json)

  def Functions(self):
    return dict(inspect.getmembers(self, inspect.ismethod))


class Login(BaseHandler):
  """Handles the login screen."""

  def get(self):  # pylint: disable-msg=C6409
    return self.render('login.html',
                       next=self.get_argument('next', '/'),
                       err=self.get_argument('err', None))

  def post(self):  # pylint: disable-msg=C6409
    username = self.get_argument('username', '')
    password = self.get_argument('password', '')

    auth = False
    if self._db.users.find({'user': username, 'password': password}).count():
      auth = True

    if auth:
      print 'Setting current user to: % s ' % username
      self.set_current_user(username)
      print 'Set current user: %s ' % self.get_current_user()
      self.redirect(self.get_argument('next', '/'))
    else:
      err = u'?err=' + tornado.escape.url_escape(
          'Incorrect username or password!')
      self.redirect(u'/login' + err)

  def set_current_user(self, user):  # pylint: disable-msg=C6409
    if user:
      self.set_secure_cookie('user',
                             tornado.escape.json_encode(user), expires_days=1)
    else:
      self.clear_cookie('user')


class Logout(BaseHandler):
  def get(self):  # pylint: disable-msg=C6409
    self.clear_cookie('user')
    self.redirect(u'/login')


class AgentUpdate(BaseHandler):
  """Metrics page handler."""

  def post(self, protocol, agent):  # pylint: disable-msg=C6409
    """Incoming message from an agent."""
    reply = self.request.body

    instance = self.request.headers.get('Instance', 'Unknown')

    if agent == 'MembranePotential':
      # This is a huge reply, DONT unpack it now, let the client unpack.
      # Just unzip.  If this gets slow, consider passing the load to the client
      if self.request.headers.get('Content-Encoding', None) == 'gzip':
        buff = StringIO.StringIO(reply)
        with gzip.GzipFile(fileobj=buff) as gzip_file:
          reply = gzip_file.read()
      for stream in self._cluster.GetViewers(instance, agent):
        stream.Send('Agent', agent, bytearray(reply),
                    protocol=protocol, translate_payload=False)

    elif agent == 'NeuronState':

      if self.request.headers.get('Content-Encoding', None) == 'gzip':
        buff = StringIO.StringIO(reply)
        gzip_file = gzip.GzipFile(fileobj=buff)
        reply = gzip_file.read()
        gzip_file.close()

      unpacked_reply = msgpack.unpackb(reply)

      for uuid in unpacked_reply:
        for stream in self._cluster.GetViewers(instance, agent,
                                               uuid):
          stream.Send('Agent', agent,
                      {'uuid': uuid, 'state': unpacked_reply[uuid]},
                      protocol='msgpack')


class MockHome(BaseHandler):

  @web.authenticated
  def get(self, path):  # pylint: disable-msg=C6409,W0613
    self.render('bigbrain.html', nav='home', functions=self.Functions())


class MockSocial(BaseHandler):

  @web.authenticated
  def get(self, path):  # pylint: disable-msg=C6409,W0613
    self.render('bigbrain.html', nav='social', functions=self.Functions())


class MockExperiments(BaseHandler):
  """Experiments page."""

  def GetExpts(self):
    expttype = self.get_argument('type', 'all')
    return self._rc.ListExperiments(self.get_current_user(), expttype)

  def ActiveTab(self):
    return self.get_argument('type', 'all')

  def GetExptName(self):
    expt_id = self.get_argument('id', None)
    if not expt_id:
      return None
    expt = self._rc.GetExperiment(self.get_current_user(), expt_id)
    return expt['name']

  def GetIDLink(self):
    expt_id = self.get_argument('id', None)
    return 'id=%s' % expt_id

  def GetID(self):
    expt_id = self.get_argument('id', None)
    return '%s' % expt_id

  def ListExperiments(self):
    self.render('bigbrain.html', nav='experiments', action='list',
                exptname=self.GetExptName(), functions=self.Functions())

  def Status(self, subpath):  # pylint: disable-msg=W0613
    self.render('bigbrain.html', nav='experiments', action='view',
                idlink=self.GetIDLink(), section='status',
                exptname=self.GetExptName(), functions=self.Functions())

  def Edit(self, subpath):  # pylint: disable-msg=W0613
    expt_id = self.get_argument('id', None)
    expt = self._rc.GetExperiment(self._user, expt_id)
    self.render('bigbrain.html', nav='experiments', action='view',
                idlink=self.GetIDLink(), section='edit',
                exptname=self.GetExptName(), functions=self.Functions(),
                code=expt.get('code', ''))

  def Parameters(self, subpath):  # pylint: disable-msg=W0613
    expt_id = self.get_argument('id', None)
    expt = self._rc.GetExperiment(self._user, expt_id)
    parameters = expt.get('parameters', [])
    self.render('bigbrain.html', nav='experiments', action='view',
                idlink=self.GetIDLink(), section='parameters',
                exptname=self.GetExptName(), functions=self.Functions(),
                parameters=parameters)

  def Compile(self, subpath):  # pylint: disable-msg=W0613
    expt_id = self.get_argument('id', None)
    compilation = self._rc.CompileExperiment(self._user, expt_id)
    warnings = compilation['warnings']
    errors = compilation['errors']
    success = True
    if errors:
      success = False

    self.render('bigbrain.html', nav='experiments', action='view',
                idlink=self.GetIDLink(), section='compile',
                exptname=self.GetExptName(), functions=self.Functions(),
                success=success, warnings=warnings, errors=errors)

  def Execute(self, subpath):  # pylint: disable-msg=W0613
    """Execute page."""
    expt_id = self.get_argument('id', None)

    expt = self._rc.GetExperiment(self.get_current_user(), expt_id)

    dimensions = []
    parameters = self._rc.GetParameters(self.get_current_user(), expt_id)
    for p in parameters['parameters']:
      if not p['enabled']:
        continue
      num_instances = len(list(bigbrain.resourcecontroller.floatrange(
          p['min'], p['max'], p['step'])))
      dimensions.append((p['name'], num_instances))

    instances = self._rc.GetExperimentInstances(expt_id)

    counts = {
        'total': 0,
        'starting': 0,
        'running': 0,
        'terminated': 0,
    }
    for i in instances:
      if i['status'].lower() == 'running':
        counts['running'] += 1
      elif i['status'].lower() == 'starting' or (
          i['status'].lower() == 'ready'):
        counts['starting'] += 1
      elif i['status'].lower() == 'terminated' or (
          i['status'].lower() == 'stopped'):
        counts['terminated'] += 1

    counts['total'] = sum([counts[x] for x in counts.keys()])

    self.render('bigbrain.html', nav='experiments', action='view',
                idlink=self.GetIDLink(), section='execute',
                exptname=expt['name'], functions=self.Functions(),
                compiled=expt['compilation']['success'], counts=counts,
                running=expt['running'], dimensions=dimensions)

  def Instances(self, subpath):  # pylint: disable-msg=W0613
    """List instances page."""
    filt = []
    parameters = []
    filtpath = []
    instances = []

    expt_id = self.get_argument('id', None)
    watch = self.get_argument('watch', None)
    expt = self._rc.GetExperiment(self.get_current_user(), expt_id)
    list_all = self.get_argument('all', None)

    if not watch:
      if not list_all:
        choosable_params = self._rc.GetParameters(self.get_current_user(),
                                                  expt_id)

        # This should be done somewhere else
        parameters = []
        for p in choosable_params['parameters']:
          if not p['enabled']:
            continue
          parameters.append({
              'name': p['name'],
              'values': list(bigbrain.resourcecontroller.floatrange(
                  p['min'], p['max'], p['step']))
          })

        filtpath = self.get_argument('filter', '')
        filt = [x.partition('=')[::2] for x in filtpath.split(',')]
        selected_filters = [y[0] for y in filt];
        parameters = [x for x in parameters if x['name'] not in
                      selected_filters]

        if not parameters:
          # We've filtered as far as we can go, show the instance
          instances = self._rc.FindExperimentInstance(expt_id, filt)

    self.render('bigbrain.html', nav='experiments', action='view',
                idlink=self.GetIDLink(), section='instances',
                exptname=self.GetExptName(), functions=self.Functions(),
                running=expt['running'], parameters=parameters, filt=filt,
                filtpath=filtpath, instances=instances, all=all, watch=watch)

  def RecordedData(self, subpath):  # pylint: disable-msg=W0613
    self.render('bigbrain.html', nav='experiments', action='view',
                idlink=self.GetIDLink(), section='recordeddata',
                exptname=self.GetExptName(), functions=self.Functions())

  def Share(self, subpath):  # pylint: disable-msg=W0613
    self.render('bigbrain.html', nav='experiments', action='view',
                idlink=self.GetIDLink(), section='share',
                exptname=self.GetExptName(), functions=self.Functions())

  @web.authenticated
  def get(self, path=None):  # pylint: disable-msg=C6409,W0613
    """Base handler for the entire site."""
    subpath = []

    if path:
      for field in path.split('/'):
        subpath.append(field)
      if not subpath[-1]:
        del subpath[-1]

    section = 'status'
    if len(subpath) >= 2:
      section = subpath[1]

    if not subpath:
      self.ListExperiments()
      return

    if section == 'status':
      self.Status(subpath)
    elif section == 'edit':
      self.Edit(subpath)
    elif section == 'parameters':
      self.Parameters(subpath)
    elif section == 'compile':
      self.Compile(subpath)
    elif section == 'execute':
      self.Execute(subpath)
    elif section == 'instances':
      self.Instances(subpath)
    elif section == 'recordeddata':
      self.RecordedData(subpath)
    elif section == 'share':
      self.Share(subpath)
    else:
      self.write('Experiment Fallthrough')

  def EditorAjax(self, action):
    if action == 'save':
      expt_id = self.get_argument('id', None)
      if not id:
        self.send_error(406)
        return
      content = self.request.body
      self._rc.SaveExperimentCode(self._user, expt_id, content)

  def ParameterAjax(self, action):
    if action == 'list':
      expt_id = self.get_argument('id', None)
      expt = self._rc.GetExperiment(self._user, expt_id)
      parameters = expt.get('parameters', [])
      self.write(json.dumps(parameters))
    elif action == 'add':
      expt_id = self.get_argument('id', None)
      parameter = json.loads(self.request.body)
      self._rc.AddParameter(self._user, expt_id, parameter)
    elif action == 'delete':
      expt_id = self.get_argument('id', None)
      parameter = json.loads(self.request.body)
      self._rc.DeleteParameter(self._user, expt_id, parameter)
    elif action == 'update':
      expt_id = self.get_argument('id', None)
      parameter = json.loads(self.request.body)
      self._rc.UpdateParameter(self._user, expt_id, parameter)
    else:
      self.send_error(404)

  def ExecuteAjax(self, action):
    if action == 'quickstatus':
      expt_id = self.get_argument('id', None)
      instances = self._rc.GetExperimentInstances(expt_id)

      counts = {
          'total': 0,
          'starting': 0,
          'running': 0,
          'terminated': 0,
      }
      for i in instances:
        if i['status'].lower() == 'running':
          counts['running'] += 1
        elif i['status'].lower() == 'starting':
          counts['starting'] += 1
        elif i['status'].lower() == 'terminated' or (
            i['status'].lower() == 'stopped'):
          counts['terminated'] += 1

      counts['total'] = sum([counts[x] for x in counts.keys()])
      self.write(json.dumps(counts))
    elif action == 'execute':
      expt_id = self.get_argument('id', None)
      self._rc.ExecuteExperiment(expt_id)
    elif action == 'stop':
      expt_id = self.get_argument('id', None)
      self._rc.StopExperiment(expt_id)
    else:
      self.send_error(404)

  def AddNewExperiment(self, payload):
    self._rc.AddExperiment(self.get_current_user(), payload)

  @web.authenticated
  def post(self, path):  # pylint: disable-msg=C6409

    if not path:
      self.send_error(405)
      return

    directive = path.split('/')

    if directive[0] == 'ajax':
      if directive[1] == 'addnew':
        self.AddNewExperiment(json.loads(self.request.body))
      if directive[1] == 'editor':
        self.EditorAjax(directive[2])
      if directive[1] == 'parameters':
        self.ParameterAjax(directive[2])
      if directive[1] == 'execute':
        self.ExecuteAjax(directive[2])
    else:
      self.send_error(404)


def InstanceHeartbeat(cluster, rpc, rc):  # pylint: disable-msg=W0613
  cluster.Heartbeat()
  for failed_instance in cluster.GetFailed():
    # Notify clients
    status = cluster.GetStatus(failed_instance)
    cluster.TearDownInstance(failed_instance)
    for stream in cluster.GetViewers(failed_instance[0], 'NetworkStream'):
      stream.Send('InstanceFailed', {}, protocol='msgpack')
    cluster.PurgeViewers(failed_instance[0])

    rc.UpdateInstanceStatus(failed_instance, status)


def Main():
  """Entrant function."""
  parser = optparse.OptionParser()
  parser.add_option('-c', '--cluster', metavar='CLUSTER_MANAGER',
                    action='store', type='string', dest='cluster',
                    help='specify cluster manager', default='GoogleCompute')
  parser.add_option('-p', '--http-port', metavar='PORT_NUM',
                    action='store', type='string', dest='http_port',
                    help='port to start http server on', default='12345')
  parser.add_option('-P', '--sim-port', metavar='SIMULATOR_PORT_NUM',
                    action='store', type='string', dest='bigbrain_port',
                    help='port that bigbrain instances run on', default='32123')
  parser.add_option('-b', '--heartbeat', metavar='SECONDS',
                    action='store', type='float', dest='heartbeat_timeout',
                    help='duration between cluster manager heartbeats',
                    default='3.0')
  (options, args) = parser.parse_args()  # pylint: disable-msg=W0612

  # Spin up database
  dbconn = pymongo.connection.Connection()
  database = dbconn.bigbrain

  # Initialize Cluster Manager
  cluster_type = {'Localhost': lcm.LocalhostClusterManager,
                  'GoogleCompute': gccm.GoogleComputeClusterManager}

  if options.cluster not in cluster_type:
    print 'Unable to instantiate cluster manager of type "%s"' % (
        options.cluster)
    sys.exit(2)

  print 'Initializing ' + options.cluster
  cluster_manager = cluster_type[options.cluster]()

  cluster_manager.Initialize(socket.gethostname(), options.http_port,
                             options.bigbrain_port)
  cluster_manager.Heartbeat()

  # Initialize RPC engine
  rpc_manager = bigbrain.rpc.AsyncSimulatorRPC(
      cluster_manager, options.bigbrain_port, 'msgpack', True)

  # Start up resource controller
  resource_controller = bigbrain.resourcecontroller.ResourceController(
      cluster_manager, rpc_manager, database)

  # Set up web handlers
  handler_params = dict(db=database, cluster=cluster_manager, rpc=rpc_manager,
                        rc=resource_controller)

  url_handlers = [
      (r'/agent/(msgpack|json)/(.*)', AgentUpdate, handler_params),
      (r'/viewstream', viewstream.ViewStream, handler_params),
      (r'/experiments', MockExperiments, handler_params),
      (r'/experiments/(.*)', MockExperiments, handler_params),
      (r'/social/?(.*)', MockSocial, handler_params),
      (r'/static/(.*)', web.StaticFileHandler, {'path': './static'}),
      (r'/login', Login, handler_params),
      (r'/logout', Logout, handler_params),
      (r'/(.*)', MockHome, handler_params)]

  application = web.Application(
      url_handlers,
      template_path='./templates/',
      cookie_secret='batsbatsbatsEVERYWHEREB@Tbot',
      debug=True)
  application.listen(options.http_port)

  def InstanceHeartbeatCallback():
    InstanceHeartbeat(cluster_manager, rpc_manager, resource_controller)
    ioloop.IOLoop.instance().add_timeout(
        time.time() + options.heartbeat_timeout, InstanceHeartbeatCallback)

  ioloop.IOLoop.instance().add_timeout(
      time.time() + options.heartbeat_timeout, InstanceHeartbeatCallback)
  ioloop.IOLoop.instance().start()
  logging.debug('Server shutdown')


if __name__ == '__main__':
  Main()
