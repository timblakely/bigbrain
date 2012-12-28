"""Base class for streams, handling the basics of BigBrain protocol."""

import json
import re

import msgpack
import tornado.escape
import tornado.websocket


class Stream(tornado.websocket.WebSocketHandler):
  """Websocket connection that is generated when a client connects."""

  def initialize(self, db, cluster, rpc, rc):  # pylint: disable-msg=C6409
    self._rpc = rpc
    self._db = db
    self._cluster = cluster
    self._rc = rc

  def get_current_user(self):  # pylint: disable-msg=C6409
    user_json = self.get_secure_cookie('user')
    if user_json:
      return tornado.escape.json_decode(user_json)
    else:
      return None

  def open(self):  # pylint: disable-msg=C6409
    self._user = self.get_current_user()
    self.Open()

  def Open(self):
    """Dummy function to be inherited."""
    pass

  def on_close(self):  # pylint: disable-msg=C6409
    """Callback when client disconnects."""
    self.Close()

  def Close(self):
    """Dummy function to be inherited."""
    pass

  def Query(self, name, params=None):
    """Handles single queries and passes them to RPC agent."""
    print '%s queried: %s' % (self.get_current_user(), name)
    # TODO(blakely): parameter checking on RPC

    # this might be better as a straight up param
    if 'instance' not in params:
      print 'Instance not given for query "%s"' % name
      return

    def Reply(reply):
      self.Send('Query', name, reply)
    self._rpc.CallQuery(params['instance'], name, params, Reply)

  def Agent(self, name, params=None):
    """Handles agent requests and passes them to appropriate instances."""
    print '%s requested agent function: %s' % (self.get_current_user(), name)
    # TODO(blakely): parameter checking on RPC

    # this might be better as a straight up param
    if 'instance' not in params:
      print 'Instance not given for query "%s"' % name
      return

    def Reply(reply):
      """Callback handler."""
      if hasattr(self, name):
        childfunc = getattr(self, name)
        childfunc(params['type'], params['instance'], params)

    self._rpc.CallAgents(params['instance'], name, params, Reply)

  def on_message(self, message):  # pylint: disable-msg=C6409
    """Callback when client sends a message via the websocket."""
    matches = re.match('(?P<type>[^:]*):(?P<protocol>msgpack|json):'
                       '(?P<name>.*):(?P<payload>.*)', message, re.DOTALL)
    msg_type = matches.group('type')
    protocol = matches.group('protocol')
    name = matches.group('name')
    payload = None
    if protocol == 'json':
      payload = json.loads(matches.group('payload'))
    elif protocol == 'msgpack':
      payload = msgpack.unpackb(matches.group('payload'))

    if not hasattr(self, msg_type):
      return

    stream_func = getattr(self, msg_type)
    stream_func(name, payload)

  def Send(self, msg_type, name, params=None, protocol='msgpack',
           translate_payload=True):
    """Sends a message to the client."""

    if isinstance(params, str):
      protocol = 'string'
    msg = bytearray('%s:%s:%s:' % (msg_type, protocol.encode('ascii'), name.encode('ascii')))

    if translate_payload:
      if protocol == 'json':
        msg.extend(json.dumps(params))
      elif protocol == 'msgpack':
        msg.extend(msgpack.packb(params))
      elif protocol == 'string':
        msg.extend(params)
    elif params:
      msg.extend(params)
    try:
      self.write_message(str(msg), binary=True)
    except IOError:
      # Broken pipe while we were writing something, that's okay, just ignore it
      pass
