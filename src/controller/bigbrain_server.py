import json
import socket
import threading

import agents
import apilayer
import msgpack
import queries
import tornado.httpclient as httpclient
import tornado.ioloop as ioloop
import tornado.web as web


class BaseHandler(web.RequestHandler):
  def initialize(self, rpc=None, css=None):
    self._rpc = rpc
    self._css = css
    self._param_mappings = {
        'float': [float],
        'double': [float],
        'int': [int],
        'string': [unicode, str]
    }

  def _MissingParams(self, cmd, params):
    missing_params = []
    required_params = self._rpc.ParamsXL(cmd)
    for p in required_params:
      if p not in params:
        missing_params.append(p)
    return missing_params

  def _CheckParamTypes(self, cmd, params):
    invalid_params = {}
    valid_params = {}
    required_params = self._rpc.ParamsXL(cmd)
    for p in required_params:
      if required_params[p] not in self._param_mappings:
        raise TypeError('%s: %s is not a known type!' % (p, required_params[p]))
      value = params[p]
      result = None
      for typecast in self._param_mappings[required_params[p]]:
        if isinstance(value, typecast):
          result = value
          break
        try:
          result = typecast(value)
          break
        except ValueError:
          pass
      if result is None:
        invalid_params[p] = required_params[p]
        continue
      valid_params[p] = result

    return (valid_params, invalid_params)

  def _IsValidRequest(self, cmd, params):
    err = {}
    if not cmd:
      err = {
          'err': 'No XLRPC given'
      }
      self.write(json.dumps(err))
      return None
    if not self._rpc.IsXLRPC(cmd):
      err = {
          'err': 'XLRPC "%s" not found' % cmd
      }
      self.write(json.dumps(err))
      return None

    missing_params = self._MissingParams(cmd, params)
    if missing_params:
      err = {
          'err': 'Missing parameters for XLRPC %s: %s' % (cmd, missing_params)
      }
      self.write(json.dumps(err))
      return None
    good_params, bad_params = self._CheckParamTypes(cmd, params)
    if bad_params:
      err = {
          'err': 'Invalid params for XLRPC %s: %s' % (cmd, bad_params)
      }
      self.write(json.dumps(err))
      return None
    return good_params

  def _GetParamsFromURL(self):
    result = {}
    for param in self.request.arguments.keys():
      result[param] = self.get_argument(param)
    return result

  def _GetParamsFromJSONBody(self):
    result = json.loads(self.request.body)
    return result

  def _GetParamsFromMsgPackBody(self):
    result = msgpack.unpackb(self.request.body)
    return result


class RPCInput(BaseHandler):

  def ProcessRequest(self, protocol, cmd):
    protocol = protocol.lower()

    params = {}
    if protocol == 'url':
      params = self._GetParamsFromURL()
    elif protocol == 'json':
      params = self._GetParamsFromJSONBody()
    elif protocol == 'msgpack':
      params = self._GetParamsFromMsgPackBody()

    valid_params = self._IsValidRequest(cmd, params)
    if not valid_params:
      print 'Invalid params'
      return

    if not self.request.headers.get('Async', False):
      self._rpc.CallXL(cmd, **valid_params)
    else:
      func = self._rpc.CallXL

      class SimpleThread(threading.Thread):
        def run(self):
          func(cmd, **valid_params)
      t = SimpleThread()
      t.start()

  def get(self, protocol=None, cmd=None):
    if protocol and not cmd:
      cmd = protocol
      protocol = 'url'
    self.ProcessRequest(protocol, cmd)

  def post(self, protocol=None, cmd=None):
    self.get(protocol, cmd)


class InfoPage(web.RequestHandler):

  def initialize(self, rpc=None, css=None):
    self._rpc = rpc
    self._css = css

  def get(self):
    self.write('<B>BigBrain</B> Interface<br>')
    self.write('<a href="/_ah/shutdown">Shutdown</a><BR>')
    self.write('<BR><B>Known RPCs:</B><BR>')
    for name in self._rpc.KnownRPCs():
      self.write('<i>%s</i><BR>' % name)
      self.write('&nbsp;&nbsp;Params:<BR>')
      params = self._rpc.ParamsXL(name)
      if not params:
        self.write('&nbsp;&nbsp;&nbsp;&nbsp;None<BR>')
      else:
        for a, b in params.iteritems():
          self.write('&nbsp;&nbsp;&nbsp;&nbsp;%s: %s<BR>' % (a, b))
      self.write('<BR>')

  def post(self):
    self.get()


class XLRPC(object):
  def __init__(self, xl_obj, class_name):
    self._func_map = {}

    self._xl_obj = xl_obj
    module = getattr(apilayer, class_name)
    for class_method in module.__dict__:
      if class_method[:5] != 'Call_':
        continue
      method = class_method[5:]
      print '>> Method: %s' % method
      func = getattr(self._xl_obj, class_method)
      if ('Params_%s' % method) not in module.__dict__:
        raise AttributeError('"Params_%s" is not defined' % method)
      params = getattr(self._xl_obj, 'Params_%s' % method)
      self._func_map[method] = {
          'params': params,
          'func': func
      }
    print 'Discovering static C declarations'
    for field in apilayer.__dict__:
      if field[:7] != 'BBCall_':
        continue
      method = field[7:]
      print '>> Static: %s' % method
      if ('BBParams_%s' % method) not in apilayer.__dict__:
        raise AttributeError('"BBParams_%s" is not defined' % method)
      func = getattr(apilayer, field)
      params = getattr(apilayer, 'BBParams_%s' % method)
      self._func_map[method] = {
          'params': params,
          'func': func
      }

  def KnownRPCs(self):
    return self._func_map.keys()

  def IsXLRPC(self, cmd):
    return cmd in self._func_map

  def ParamsXL(self, cmd):
    if not self.IsXLRPC(cmd):
      return None
    return self._func_map[cmd]['params']()

  def CallXL(self, cmd, *argv, **kwargs):
    if cmd not in self._func_map:
      print 'Unknown XL function %s' % cmd
      return
    f = self._func_map[cmd]['func']
    if argv:
      return f(*argv)
    elif kwargs:
      return f(**kwargs)
    else:
      return f()


class Shutdown(web.RequestHandler):

  def get(self):
    self.write('Shutting down...')
    ioloop.IOLoop.instance().stop()

  def post(self):
    self.get()


class Agents(BaseHandler):
  def ProcessRequest(self, protocol, cmd):
    protocol = protocol.lower()

    params = {}
    if protocol == 'url':
      params = self._GetParamsFromURL()
    elif protocol == 'json':
      params = self._GetParamsFromJSONBody()
    elif protocol == 'msgpack':
      params = self._GetParamsFromMsgPackBody()

    if cmd.lower() == 'startagent':
      if 'type' not in params:
        err = {
            'err': 'Need "type" to start agent'
        }
        self.write(json.dumps(err))
        return
      self._css.AddAgent(params['type'])
    elif cmd.lower() == 'stopagent':
      if 'type' not in params:
        err = {
            'err': 'Need "type" to stop agent'
        }
        self.write(json.dumps(err))
        return
      self._css.RemoveAgent(params['type'])
    elif cmd.lower() == 'adduuidstoagent':
      if 'type' not in params:
        err = {
            'err': 'Need "type" to add uuids to agent'
        }
        self.write(json.dumps(err))
        return
      if 'uuids' not in params:
        err = {
            'err': 'Need "uuids" to remove from agent %s' % params['type']
        }
        self.write(json.dumps(err))
        return
      self._css.AddIDsToAgent(params['type'], params['uuids'])

  def get(self, protocol=None, cmd=None):
    if protocol and not cmd:
      cmd = protocol
      protocol = 'url'
    self.ProcessRequest(protocol, cmd)

  def post(self, protocol=None, cmd=None):
    self.get(protocol, cmd)


class Queries(BaseHandler):
  def ProcessRequest(self, protocol, cmd):
    protocol = protocol.lower()

    params = {}
    if protocol == 'url':
      params = self._GetParamsFromURL()
    elif protocol == 'json':
      params = self._GetParamsFromJSONBody()
    elif protocol == 'msgpack':
      params = self._GetParamsFromMsgPackBody()

    try:
      query = getattr(queries, cmd)
    except AttributeError:
      query = {}
    result = {}
    if query:
      result = query(self._rpc).Ask(params)

    if protocol == 'json':
      reply = json.dumps(result)
    elif protocol == 'msgpack':
      reply = msgpack.packb(result)
    self.write(reply)

  def get(self, protocol=None, cmd=None):
    if protocol and not cmd:
      cmd = protocol
      protocol = 'url'
    self.ProcessRequest(protocol, cmd)

  def post(self, protocol=None, cmd=None):
    self.get(protocol, cmd)


class BigBrainClientSideServer(threading.Thread):
  def Initialize(self):
    print 'Initializing CAPI function map'
    for field in apilayer.__dict__:
      if field[:8] == 'BBClass_':
        print 'Found class: %s' % field[8:]
        interface_class = getattr(apilayer, field)
        self._xl_obj = interface_class()
        self.class_name = field
        break

    self._agents = {}
    self._agentlock = threading.Lock()

  def GetBBClass(self):
    return self._xl_obj

  def IsRunning(self):
    return ioloop.IOLoop.instance().running()

  def AddAgent(self, name):
    try:
      agent = getattr(agents, name)
    except AttributeError:
      print 'Agent %s not a known agent type' % name
      return
    a = agent(self._rpc, self._instance_name, self._srv_host, self._srv_port,
              async=False)
    with self._agentlock:
      self._agents[name] = a

  def AddIDsToAgent(self, name, uuids):
    with self._agentlock:
      if name not in self._agents:
        return
      for uuid in uuids:
        self._agents[name].AddID(uuid)

  def ProcessAgents(self):
    # This could trivially be multithreaded
    # Consider it a TODO(blakely)
    with self._agentlock:
      for name in self._agents:
        self._agents[name].Update()

  def RemoveAgent(self, name):
    if name not in self._agents:
      print 'Cannot remove unknown agent %s' % name
      return
    with self._agentlock:
      del self._agents[name]

  def SetInstanceName(self, name):
    if name == '__AUTO__':
      http_client = httpclient.HTTPClient()
      try:
        self._instance_name = http_client.fetch(
            'http://metadata.google.internal/0.1/meta-data/attributes/'
            'bb_instance_name'
        ).body
      except httpclient.HTTPError:
        self._instance_name = '__METADATA_FAIL__'
      except socket.error:
        self._name = '__SOCKET_FAIL__'
    else:
      self._instance_name = name
    print 'BBInstance name: %s' % self._instance_name

  def SetInstancePort(self, name):
    if name == '__AUTO__':
      http_client = httpclient.HTTPClient()
      try:
        self._instance_port = http_client.fetch(
            'http://metadata.google.internal/0.1/meta-data/attributes/'
            'bb_instance_port'
        ).body
      except httpclient.HTTPError:
        self._instance_name = '__METADATA_FAIL__'
      except socket.error:
        self._srv_host = '__SOCKET_FAIL__'
    else:
      self._instance_port = name
    print 'BBInstance port: %s' % self._instance_port

  def SetSrvHost(self, host):
    if host == '__AUTO__':
      http_client = httpclient.HTTPClient()
      try:
        self._srv_host = http_client.fetch(
            'http://metadata.google.internal/0.1/meta-data/attributes/'
            'bb_srv_host'
        ).body
      except httpclient.HTTPError:
        self._srv_host = '__METADATA_FAIL__'
      except socket.error:
        self._srv_host = '__SOCKET_FAIL__'
    else:
      self._srv_host = host
    print 'Registered BBServer host as %s' % self._srv_host

  def SetSrvPort(self, port):
    if port == '__AUTO__':
      http_client = httpclient.HTTPClient()
      try:
        self._srv_port = http_client.fetch(
            'http://metadata.google.internal/0.1/meta-data/attributes/'
            'bb_srv_port'
        ).body
      except httpclient.HTTPError:
        self._srv_port = '__METADATA_FAIL__'
      except socket.error:
        self._srv_port = '__SOCKET_FAIL__'
    else:
      self._srv_port = port
    print 'Registered BBServer port as %s' % self._srv_port

  def run(self):
    rpc = XLRPC(self._xl_obj, self.class_name)
    self._rpc = rpc

    params = dict(rpc=rpc, css=self)

    url_handlers = [
        (r'/_ah/shutdown', Shutdown),
        (r'/rpc/([^/]*)/?(.*)', RPCInput, params),
        (r'/agents/([^/]*)/?(.*)', Agents, params),
        (r'/query/([^/]*)/?(.*)', Queries, params),
        (r'/(favicon.ico)', web.StaticFileHandler,
            {'path': '../bigbrain/static'}),
        (r'/.*', InfoPage, params)]

    application = web.Application(
        url_handlers,
        template_path='./templates/',
        cookie_secret='batsbatsbatsEVERYWHEREB@Tbot',
        debug=False)
    application.listen(self._instance_port)
    ioloop.IOLoop.instance().start()

  def Shutdown(self):
    ioloop.IOLoop.instance().stop()

if __name__ == '__main__':
  print 'In Main'
else:
  print '< BigBrain python module loaded >'
