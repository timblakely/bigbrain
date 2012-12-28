

class BigBrainQuery(object):
  def __init__(self, rpc):
    self._rpc = rpc

  def Ask(self):
    raise NotImplementedError('Query did not implement "Ask"')


class CellLocations(BigBrainQuery):
  def Ask(self, params):
    answer = {}
    uuids = self._rpc.CallXL('GetNeuronUUIDs')
    for uuid in uuids:
      pos = self._rpc.CallXL('GetCellPosition', uuid=uuid)
      if not pos:
        continue
      answer[uuid] = {
          'uuid': uuid,
          'x': pos[0],
          'y': pos[1],
          'z': pos[2]
      }
    return answer


class CellConnections(BigBrainQuery):
  def Ask(self, params):

    if 'uuid' not in params:
      return

    targets = self._rpc.CallXL('GetCellConnections', uuid=params['uuid'])
    answer = {
        'source': params['uuid'],
        'targets': targets
    }
    return answer


class CellParameters(BigBrainQuery):
  def Ask(self, params):
    if 'uuid' not in params:
      return

    names = self._rpc.CallXL('GetCellParamNames', uuid=params['uuid'])
    vals = self._rpc.CallXL('GetCellParamValues', uuid=params['uuid'])

    return dict(zip(names, vals))

