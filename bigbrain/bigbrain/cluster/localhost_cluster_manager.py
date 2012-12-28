"""Localhost implementation of ClusterManager."""

import logging
import os
import signal
import subprocess
from cluster_manager import BigBrainClusterManager
from cluster_manager import InstanceNotFound


class LocalhostClusterManager(BigBrainClusterManager):
  """Instance of LocalhostClusterManager used by the front-end."""

  def Initialize(self, front_end_hostname, front_end_port, simulator_port):
    """Start up LocalhostClusterManager."""
    self._max_num_processes = 50
    self._proc = None
    self._pid = None
    self._front_end_host = front_end_hostname
    self._front_end_port = front_end_port
    self._simulator_port = simulator_port
    self._instances = {}

    # TODO(blakely): change os.popen to subprocess.check_output
    # TODO(blakely): force ps to list specific fields, independent of order

    for line in os.popen('ps xa'):
      fields = line.split()
      pid = fields[0]
      process = fields[4]
      if process.find('simulator') > 0:
        name = 'localhost_' + pid
        port = fields[fields.index('-p') + 1]
        self._AddInstanceToMap(
            name, None, int(pid), port, 'RUNNING', True, True, False, False)

  def ListInstanceNames(self):
    return self._instances.keys()

  def NumInstances(self):
    return len(self._instances)

  def MaxNumInstances(self):
    return self._max_num_processes

  def StartNewInstance(self):
    """Starts a new process locally."""
    if len(self._instances) == self._max_num_processes:
      return None

    sequential_port = int(self._simulator_port) + len(self._instances)
    proc = subprocess.Popen(
        [
            '../bin/simulator', '-n', 'localhost', '-F',
            self._front_end_host, '-P', self._front_end_port, '-p',
            str(sequential_port)
        ],
        cwd='../bin/'
    )
    name = 'localhost_' + str(proc.pid)
    self._AddInstanceToMap(
        name, proc, int(proc.pid), sequential_port, 'RUNNING', True, True,
        False, False
    )
    return name

  def GetInstanceHost(self, name=None):
    """Returns the hostname of the GCE instance of name 'name'."""

    if not name:
      return dict((k, a['host'] + ':' + str(a['port'])) for k, a in
                  self._instances.iteritems())
    if isinstance(name, str):
      if name not in self._instances:
        raise InstanceNotFound()
      return '%s:%s' % (self._instances[name]['host'],
                        self._instances[name]['port'])
    return dict((k, self._instances[k]['host'] + ':' +
                 self._instances[name]['port'])
                for k in name if k in self._instances)

  def GetStatus(self, requested_instances=None):
    return self._GetAttribFromInstanceMap(requested_instances, 'status')

  def IsStarted(self, requested_instances=None):
    return self._GetAttribFromInstanceMap(requested_instances, 'started')

  def IsRunning(self, requested_instances=None):
    return self._GetAttribFromInstanceMap(requested_instances, 'running')

  def IsFailed(self, requested_instances=None):
    return self._GetAttribFromInstanceMap(requested_instances, 'failed')

  def GetFailed(self):
    return dict((k, a['status']) for k, a in self._instances.iteritems() if
                a['failed'])

  def Heartbeat(self):
    """Checks if processess have failed or are dead."""
    running = {}
    for line in os.popen('ps xa'):
      fields = line.split()
      pid = fields[0]
      process = fields[4]
      if process.find('simulator') > 0 and len(fields) > 6:
        name = 'localhost_' + pid
        running[name] = {
            'pid': int(pid),
            'port': fields[fields.index('-p') + 1]
        }

    missing_instances = set(self._instances.keys()).difference(running.keys())

    for instance in missing_instances:
      self._instances[instance]['status'] = 'TERMINATED'
      if not self._instances[instance]['shutdown']:
        # If it was terminated before it hit 'RUNNING', mark that it failed
        self._instances[instance]['failed'] = True
      self._instances[instance]['running'] = False
      if self._instances[instance]['proc']:
        # Need to process return code so we don't have zombies
        self._instances[instance]['proc'].wait()

    # Can start some instances from command line manually
    # Helpful for debugging
    new_instances = set(running.keys()).difference(self._instances.keys())
    for instance in new_instances:
      self._AddInstanceToMap(
          instance, None, running[instance]['pid'], running[instance]['port'],
          'RUNNING', True, True, False, False)

  def TearDownInstance(self, name):
    """Terminate the requested process."""
    if name not in self._instances.keys():
      logging.warning(name + ' is not a known instance, ignoring teardown')
      return False

    try:
      os.kill(self._instances[name]['pid'], signal.SIGKILL)
    except OSError as e:
      if e.errno != 3:
        return False
      # Not found, so signal we killed it
    del self._instances[name]
    return True

  def IsReady(self, name):
    return name in self._instances

  def _AddInstanceToMap(self, name, proc, pid, port, status, started, running,
                        failed, shutdown):
    self._instances[name] = {
        'name': name,
        'proc': proc,
        'pid': pid,
        'host': 'localhost',
        'port': port,
        'status': status,
        'started': started,
        'running': running,
        'failed': failed,
        'shutdown': shutdown,
    }

  def _GetAttribFromInstanceMap(self, instance, attrib):
    if not instance:
      return dict((k, a[attrib]) for k, a in self._instances.iteritems())
    if isinstance(instance, str) or isinstance(instance, unicode):
      name = instance
      if name not in self._instances:
        raise InstanceNotFound()
      return self._instances[name][attrib]
    return dict((k, self._instances[k][attrib])
                for k in instance if k in self._instances)
