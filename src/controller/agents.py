import cStringIO
import gzip
import json

import msgpack
import tornado.httpclient as httpclient


class BigBrainAgent(object):
  def __init__(self, rpc, simulator_name, host, port, https=False,
               protocol='msgpack', gzipped=True, async=True):
    self._protocol = protocol
    self._rpc = rpc
    self._gzipped = gzipped
    self._host = host
    self._port = port
    self._sim_name = simulator_name
    self._async = async
    if https:
      self._http = 'https'
    else:
      self._http = 'http'
    self._watching_ids = set()

  def Name(self):
    raise NotImplementedError('Inheriting Agent did not implement Name')

  def _FullURL(self):
    name = self.Name()
    full_url = '%s://%s:%s/agent/%s/%s' % (
        self._http,
        self._host,
        self._port,
        self._protocol,
        name
    )
    return full_url

  def SendUpdate(self, update):

    if self._protocol == 'msgpack':
      payload = msgpack.packb(update)
    elif self._protocol == 'json':
      payload = json.dumps(update)

    if self._gzipped:
      buf = cStringIO.StringIO()
      gzip_file = gzip.GzipFile(fileobj=buf, mode='wb')
      gzip_file.write(payload)
      gzip_file.close()
      payload = buf.getvalue()

    url = self._FullURL()

    request = httpclient.HTTPRequest(url, method='POST', use_gzip=True,
                                     body=payload, binary_body=True,
                                     request_timeout=9999999)
    request.headers.add('Instance', self._sim_name)

    if self._gzipped:
      request.headers.add('Content-Encoding', 'gzip')

    if self._async:

      def LocalCallback(reply):
        pass
      client = httpclient.AsyncHTTPClient(max_clients=100)
      client.fetch(request, LocalCallback)
    else:
      client = httpclient.HTTPClient()
      client.fetch(request)

  def GetIDs(self):
    return self._watching_ids

  def AddID(self, uuid):
    self._watching_ids.add(uuid)

  def RemoveID(self, uuid):
    self._watching_ids.remove(uuid)


class MembranePotential(BigBrainAgent):
  def Name(self):
    return 'MembranePotential'

  def Update(self):
    # Could cache this for a speedup, but that could open a can of worms when
    # the network changes
    uuids = self._rpc.CallXL('GetNeuronUUIDs')
    membrane_potentials = self._rpc.CallXL('GetMembranePotentials')

    update = {
        'v': membrane_potentials,
        'uuids': uuids
    }

    self.SendUpdate(update)


class NeuronState(BigBrainAgent):
  def Name(self):
    return 'NeuronState'

  def Update(self):
    if not self.GetIDs():
      return

    update = {}

    for uuid in self.GetIDs():
      names = self._rpc.CallXL('GetCellStateNames', uuid=uuid)
      vals = self._rpc.CallXL('GetCellStateValues', uuid=uuid)
      update[uuid] = dict(zip(names, vals))

    self.SendUpdate(update)
