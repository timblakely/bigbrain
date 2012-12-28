# Copyright 2012 Google Inc. All Rights Reserved.

"""Management of a cluster running on Google Compute Engine."""

import json
import operator
import random
import string
import urllib2

from bigbrain.oauth2.apiclient.errors import HttpError
import bigbrain.cluster.cluster_manager as cluster_manager
from bigbrain.oauth2.apiclient.discovery import build
import bigbrain.oauth2.httplib2 as httplib2
from  bigbrain.oauth2.PyCryptoSignedJWT import PyCryptoSignedJwtAssertionCredentials as JWTCreds


class GoogleComputeClusterManager(cluster_manager.BigBrainClusterManager):
  """GCE implemenation of ClusterManager."""

  def Initialize(self, front_end_hostname, front_end_port, simulator_port):
    """Initialization and set parameters."""

    self._max_num_processes = 256

    self.force_lookup = False

    self._startup_script = """
#! /bin/bash
cd /home/blakely/bigbrain/bin
./simulator -n %s -F %s -P %s -p %s
"""
    self._project_id = 'google.com:bigapply-dev'
    self._network_name = 'bigbrain-testing'
    self._image_name = 'bigbrain-image-2012-11-02'
    self._instance_name = 'bigbrain-instance-'

    scopes = ['https://www.googleapis.com/auth/compute']
    key = open('bigbrain/oauth2/bigapply-privatekey.pem').read()
    credentials = JWTCreds(
        ('348022235562-01enhj0d560aldofo751dh712lij96c2'
         '@developer.gserviceaccount.com'),
        key,
        scope=' '.join(scopes))
    self._http = httplib2.Http()
    self._http = credentials.authorize(self._http)

    self._service = build(serviceName='compute', version='v1beta12',
                          http=self._http)

    self._instances = self._service.instances()
    self._machine_types = self._service.machineTypes()
    self._zones = self._service.zones()
    self._networks = self._service.networks()
    self._images = self._service.images()

    # Description of the desired instance to spin up
    self._instance_description = {}

    # Known running instances
    self._instance_map = {}

    try:
      reply = urllib2.urlopen(
          'http://metadata.google.internal/0.1/meta-data/network')
      reply = json.loads(reply.read())
      front_end_hostname = reply['networkInterface'][0]['ip']
    except urllib2.URLError:
      print 'Appears not to be running in GCE! Things may go awry...'

    self._front_end_host = front_end_hostname
    self._front_end_port = front_end_port
    self._simulator_port = simulator_port
    self._instance_description = {
        'kind': 'compute#instance',
        'description': 'BigBrain instance',
        }
    # Get CPU types
    reply = self._machine_types.list(project=self._project_id).execute()
    desired_machine_type = max(reply['items'],
                               key=operator.itemgetter('guestCpus'))

    self._instance_description['machineType'] = desired_machine_type['selfLink']
    # Get Zones
    reply = self._zones.list(project=self._project_id).execute()
    self._instance_description['zone'] = ''
    i = 0
    while reply['items'][i]['status'] == 'DOWN' or (
        reply['items'][i]['name'].find('europe') >= 0):
      i += 1
    self._instance_description['zone'] = reply['items'][i]['selfLink']
    reply = self._images.list(project=self._project_id, filter='name eq ' +
                              self._image_name).execute()
    for description in reply['items']:
      if description['name'] == self._image_name:
        self._instance_description['image'] = description['selfLink']
        break

    reply = self._networks.list(project=self._project_id,
                                filter='name eq bigbrain.*').execute()
    self._instance_description['networkInterfaces'] = []
    # ==========================================================================
    # self._instance_description['networkInterfaces'].append({
    #  'network': reply['items'][0]['selfLink'],
    #  'accessConfigs': [{
    #      'type': 'ONE_TO_ONE_NAT'
    #  }]
    # })
    # ==========================================================================

    self._instance_description['networkInterfaces'].append({
        'network': reply['items'][0]['selfLink']
    })
    return

  def ListInstanceNames(self):
    return self._instance_map.keys()

  def NumInstances(self):
    """Number of BigBrain instance up and running."""
    return len(self._instance_map)

  def MaxNumInstances(self):
    return self._max_num_processes

  def StartNewInstance(self):
    """Start a new BigBrain instance in GCE."""
    name = self._instance_name + self.__IDGenerator(size=16)
    self._custom_startup_script = self._startup_script % (
        name, self._front_end_host, self._front_end_port,
        self._simulator_port)

    self._instance_description['name'] = name
    self._instance_description['metadata'] = {
        'kind': 'compute#metadata',
        'items': [
            {
                'key': 'bb_instance_name',
                'value': name
            },
            {
                'key': 'bb_instance_port',
                'value': self._simulator_port
            },
            {
                'key': 'bb_srv_host',
                'value': self._front_end_host
            },
            {
                'key': 'bb_srv_port',
                'value': self._front_end_port
            }
        ]
    }
    self._instance_description['metadata']['items'].append({
        'key': 'startup-script',
        'value': self._custom_startup_script
    })

    self._instances.insert(project=self._project_id,
                           body=self._instance_description).execute()

    return name

  def TearDownInstance(self, name):
    if name not in self._instance_map:
      raise cluster_manager.InstanceNotFound()
    if not self._instance_map[name]['failed']:
      self._instances.delete(project=self._project_id,
                             instance=name).execute()
    del self._instance_map[name]

  def GetInstanceHost(self, name=None, full_hostname=True):
    """Returns the hostname of the GCE instance of name 'name'."""
    port = ''
    if full_hostname:
      port = ':' + self._simulator_port
    if not name:
      return dict((k, a['host'] + port) for k, a in
                  self._instance_map.iteritems())
    if isinstance(name, str):
      if name not in self._instance_map:
        raise cluster_manager.InstanceNotFound()
      return self._instance_map[name]['host'] + port
    return dict((k, self._instance_map[k]['host'] + port)
                for k in name if k in self._instance_map)

  def GetStatus(self, requested_instances=None):
    return self._GetAttribFromInstanceMap(requested_instances, 'status')

  def IsStarted(self, requested_instances=None):
    return self._GetAttribFromInstanceMap(requested_instances, 'started')

  def IsRunning(self, requested_instances=None):
    return self._GetAttribFromInstanceMap(requested_instances, 'running')

  def IsFailed(self, requested_instances=None):
    return self._GetAttribFromInstanceMap(requested_instances, 'failed')

  def GetFailed(self, requested_instances=None):  # pylint: disable-msg=W0613
    return dict((k, a['status']) for k, a in self._instance_map.iteritems() if
                a['failed'])

  def Heartbeat(self):
    """Ping the GCE server and see the status of all instances."""
    try:
      reply = self._instances.list(
          project=self._project_id,
          filter='name eq bigbrain-instance-.*').execute()
    except HttpError:
      return
    gce_instances = [x['name'] for x in reply.get('items', [])]

    missing_instances = set(self._instance_map.keys()).difference(gce_instances)

    for instance in missing_instances:
      self._instance_map[instance]['status'] = 'TERMINATED'
      if not self._instance_map[instance]['shutdown']:
        # If it was terminated before it hit 'RUNNING', mark that it failed
        self._instance_map[instance]['failed'] = True
      self._instance_map[instance]['running'] = False

    for instance in gce_instances:
      resource = next((x for x in reply.get('items', [])
                       if x['name'] == instance), None)

      name = resource['name']
      # GCE has not given this instance an IP address yet, pretend we
      # haven't seen it
      if not resource['networkInterfaces'][0].get('networkIP', None):
        continue
      ip = resource['networkInterfaces'][0]['networkIP']
      # ip = resource['networkInterfaces'][0]['accessConfigs'][0]['natIP']

      status = resource['status']

      if name not in self._instance_map:
        # Haven't seen this instance before, assume it's starting up
        self._AddInstanceToMap(
            name=name, host=ip, status=status, started=status == 'RUNNING',
            running=status == 'RUNNING', failed=False, shutdown=False)

      self._instance_map[name]['status'] = status

      if not self._instance_map[name]['started']:
        # Hasn't started up yet unless status is 'RUNNING'
        self._instance_map[name]['started'] = True

      if status == 'RUNNING' and not self._instance_map[name]['running']:
        # Started, now mark as running
        self._instance_map[name]['running'] = True

      if status == 'TERMINATED' or status == 'FAILED' or status == 'STOPPED':
        # Instance is over
        if self._instance_map[name]['running']:
          # Was running, now mark as not running
          self._instance_map[name]['running'] = False
        if not self._instance_map[name]['shutdown']:
          # Wasn't told to shutdown, so mark as failed
          self._instance_map[name]['failed'] = True

  def _AddInstanceToMap(self, name, host, status, started, running, failed,
                        shutdown):
    self._instance_map[name] = {
        'name': name,
        'host': host,
        'port': self._simulator_port,
        'status': status,
        'started': started,
        'running': running,
        'failed': failed,
        'shutdown': shutdown,
    }

  def __IDGenerator(self, size=6,
                    chars=string.ascii_lowercase + string.digits):
    """Generates a random alphaneumeric string of length 'size'."""
    return ''.join(random.choice(chars) for x in range(size))  # pylint: disable-msg=W0613,C6310

  def _GetAttribFromInstanceMap(self, instance, attrib):
    if not instance:
      return dict((k, a[attrib]) for k, a in self._instance_map.iteritems())
    if isinstance(instance, str) or isinstance(instance, unicode):
      name = instance
      if name not in self._instance_map:
        raise cluster_manager.InstanceNotFound()
      return self._instance_map[name][attrib]
    return dict((k, self._instance_map[k][attrib])
                for k in instance if k in self._instance_map)
