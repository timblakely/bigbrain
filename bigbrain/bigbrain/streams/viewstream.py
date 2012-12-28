"""Implementation of the stream that sends visual updates to client."""

from bigbrain.streams.stream import Stream


class ViewStream(Stream):
  """Streaming connection for network viewing."""

  def Open(self):  # pylint: disable-msg=C6409
    """Callback when client connects."""
    print self.get_current_user() + ' created new viewstream'
    self._agents = {}

  def Close(self):
    self._cluster.ViewStreamClosed(self)
    for instance in self._agents:
      for agent in self._agents[instance]:
        if not self._cluster.GetViewers(instance, agent):
          self._rpc.CallAgents(instance, 'StopAgent', {'type': agent})

  def StartAgent(self, agent_type, instance, params):
    self._agents.setdefault(instance, set())
    self._agents[instance].add(agent_type)

    self._cluster.AddAgentToViewer(instance, self, agent_type)

  def StopAgent(self, agent_type, instance, params):
    if agent_type in self._agents[instance]:
      self._agents[instance].remove(agent_type)
    self._cluster.RemoveAgentFromViewer(instance, self, agent_type)

  def AddUUIDsToAgent(self, agent_type, instance, params):
    self._cluster.AddAgentToViewer(instance, self, agent_type, params['uuids'])
