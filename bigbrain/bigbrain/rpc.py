# Copyright 2012 Google Inc. All Rights Reserved.

"""Handles RPC calls to simulators and performs callbacks when necessary."""

import json
import logging
import re

import msgpack

from tornado import httpclient


class AsyncSimulatorRPC(object):
  """Facilitates simulator REST requests asynchronously."""

  def __init__(self, cluster_manager, simulator_port='32123', protocol='json',
               gzipped=False):
    self._cm = cluster_manager
    self._rpc_protocol = protocol
    self._gzip = gzipped  # Not used currently
    self._simulator_port = simulator_port

  def _PrepareRPCRequest(self, call_type, instance, rpc, params=None):
    host = self._cm.GetInstanceHost(instance)
    if host is None:
      return None

    matches = re.match(
        r'(?:(?P<protocol>http[s]?)?://)?(?P<host>[^:/ ]+):?(?P<port>\d*)',
        host)
    if not matches:
      logging.error('Invalid host string')
      return None
    if not matches.group('host'):
      logging.error('Invalid host')
      return None
    http_host = matches.group('host')

    http_port = matches.group('port')
    if not http_port:
      logging.warning('No port specified, using default port %s',
                      self._simulator_port)
      http_port = self._simulator_port
    http_protocol = matches.group('protocol')
    if not http_protocol:
      logging.info('Assuming http protocol for %s', host)
      http_protocol = 'http'

    data = None
    if self._rpc_protocol == 'json' and params:
      data = json.dumps(params)
    elif self._rpc_protocol == 'msgpack' and params:
      data = bytearray(msgpack.packb(params))

    full_url = '%s://%s:%s/%s/%s/%s' % (
        http_protocol, http_host, http_port, call_type,
        self._rpc_protocol, rpc)

    # Shoehorn data into tornado's bizarre requirements
    method = 'POST' if data else 'GET'
    payload = bytes(data) if data else None

    request = httpclient.HTTPRequest(full_url, method=method, use_gzip=True,
                                     body=payload, binary_body=True,
                                     request_timeout=9999999)
    return request

  def RPCReturn(self, response, callback=None):
    """Handles RPC response and calls callback if necessary."""
    if response.error:
      logging.error('HTTP ERROR!\n>> URL: %s\n>> Code: %i',
                    response.request.url, response.code)
      return

    reply = response.buffer.getvalue()

    # NOTE: Don't need to gunzip it here, since Tornado does it for us
    # ==========================================================================
    # if self._gzip and len(reply) > 2:  #len needed for empty responses
    #  buff = StringIO(reply)
    #  gzip_file = gzip.GzipFile(fileobj=buff)
    #  reply = gzip_file.read()
    #  gzip_file.close()
    # elif self._gzip and len(reply) == 2:
    #  reply = None
    # ==========================================================================

    if self._rpc_protocol == 'json' and reply:
      try:
        unpacked_reply = json.loads(reply)
      except ValueError as err:
        logging.error(err)
        logging.error('Unable to parse JSON reply from RPC %s',
                      response.request.get_full_url())
    elif self._rpc_protocol == 'msgpack' and reply:
      unpacked_reply = msgpack.unpackb(reply)
      if reply and not unpacked_reply:
        logging.warning('Reply detected, but msgpack '
                        'parse is empty from RPC %s', response.request.url)
        logging.warning(reply)
        logging.warning(unpacked_reply)
    else:
      unpacked_reply = reply

    if callback:
      callback(unpacked_reply)

  def Call(self, call_type, instance, rpc, parameters=None, callback=None):
    """General calling function for RPCs to simulators."""
    client = httpclient.AsyncHTTPClient(max_clients=100)
    request = self._PrepareRPCRequest(call_type, instance, rpc, parameters)
    request.headers['Async'] = 'True'
    if not request:
      logging.error('Couldn\'t create RPC request')
      return None

    def LocalCallback(response):
      self.RPCReturn(response, callback)
    client.fetch(request, LocalCallback)

  def CallRPC(self, instance, rpc, parameters=None, callback=None):
    self.Call('rpc', instance, rpc, parameters, callback)

  def CallAgents(self, instance, rpc, parameters=None, callback=None):
    self.Call('agents', instance, rpc, parameters, callback)

  def CallQuery(self, instance, rpc, parameters=None, callback=None):
    self.Call('query', instance, rpc, parameters, callback)
