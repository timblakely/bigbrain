# Copyright 2012 Google Inc. All Rights Reserved.

"""Class which subclasses must inherit from to be a CM for the front-end."""

import abc
import collections


class InstanceNotFound(Exception):
  pass


class BigBrainClusterManager(object):
  """Class which subclasses must inherit from to be a CM for the front-end."""
  __metaclass__ = abc.ABCMeta

  def __init__(self, strict_heartbeat=True):
    self._viewstream_map = {}
    self._agent_map = {}
    self._strict_heartbeat = strict_heartbeat

  @abc.abstractmethod
  def Initialize(self, front_end_hostname, front_end_port, simulator_port):
    """Perform any initialization the cluster manager needs."""

  @abc.abstractmethod
  def NumInstances(self):
    """Return number of currently running instances."""

  @abc.abstractmethod
  def MaxNumInstances(self):
    """Return maximum number of currently running instances."""

  @abc.abstractmethod
  def ListInstanceNames(self):
    """Returns a list of all currently running instances."""

  @abc.abstractmethod
  def StartNewInstance(self):
    """Spin up a new bigbrain instance."""

  @abc.abstractmethod
  def GetInstanceHost(self, name):
    """Returns the host name for the given instance, with or without port."""

  @abc.abstractmethod
  def TearDownInstance(self, name):
    """Tear down instance for name 'name'."""

  @abc.abstractmethod
  def IsRunning(self, requested_instances=None):
    """Returns a boolean whether instance is ready to be connected to."""

  @abc.abstractmethod
  def IsStarted(self, requested_instances=None):
    """Returns a boolean whether instance is powered up started."""

  @abc.abstractmethod
  def IsFailed(self, requested_instances=None):
    """Returns a boolean whether instance has failed."""

  @abc.abstractmethod
  def GetFailed(self):
    """Returns a dict of name:status for failed-but-not-removed instances."""

  @abc.abstractmethod
  def GetStatus(self, requested_instances=None):
    """Gets status of instances."""

  def Heartbeat(self):
    if self._strict_heartbeat:
      raise NotImplementedError('Inherited class did not implement Heartbeat')

  def PurgeViewers(self, instance):
    if self._viewstream_map.get(instance, None):
      self._viewstream_map[instance].clear()

  def ViewStreamClosed(self, viewstream):
    for instance in self._viewstream_map:
      if viewstream in self._viewstream_map[instance]:
        del self._viewstream_map[instance][viewstream]

  def GetViewers(self, instance, stream_type, uuid='all'):
    """Gets all the viewstreams associated with this instance."""
    if self._viewstream_map.get(instance, None):

      # Definitely a more beautiful way to do this... this is slow and ugly :/
      results = []
      for vs in self._viewstream_map[instance]:
        if stream_type in self._viewstream_map[instance][vs]['types']:
          if uuid == 'all':
            results.append(vs)
          elif uuid in self._viewstream_map[instance][vs]['types'][stream_type]:
            results.append(vs)
      return results

    return []

  def AddAgentToViewer(self, instance, viewstream, agent_type, uuids=None):
    if instance not in self._viewstream_map:
      self._viewstream_map[instance] = {}
    if viewstream not in self._viewstream_map[instance]:
      self._viewstream_map[instance][viewstream] = {'types': {}}

    if agent_type not in self._viewstream_map[instance][viewstream]['types']:
      self._viewstream_map[instance][viewstream]['types'][agent_type] = set()
    if isinstance(uuids, int):
      self._viewstream_map[instance][viewstream]['types'][agent_type].add(uuids)
    elif isinstance(uuids, collections.Iterable):
      self._viewstream_map[instance][viewstream]['types'][agent_type].update(
          uuids)

  def RemoveAgentFromViewer(self, instance, viewstream, stream_type,
                            uuids='all'):  # pylint: disable-msg=W0613
    if instance not in self._viewstream_map:
      print 'Instance %s not known?!' % instance
      return
    if viewstream not in self._viewstream_map[instance]:
      print 'Viewstream not added to instance %s' % instance
      return

    self._viewstream_map[instance][viewstream]['types'].pop(
        stream_type, None)

