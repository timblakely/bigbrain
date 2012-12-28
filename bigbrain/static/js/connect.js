function Connection() {
  var t_ = this;
  var version_ = "0";
  var connected_ = false;
  var verbose_ = false;
  var socket_;
  var host_;
  var port_;
  var listeners_ = {};
  var type_str;
  var reconnect_ = false;
  
  var logger_ = function(msg) {
    console.log(msg);
  }
  
  function notify_listeners(type, event, data) {
	if(verbose_) {
		logger_('>> '+type+": "+event);
	}
    if(listeners_[type] == undefined) {
      return;
    }
    for (var i=0; i < listeners_[type].length; i++) {
   	  listeners_[type][i](event, data);
    }
  } 
  
  function on_connected() {
    connected_ = true;
    if(verbose_) { logger_(type_str + " connected!") };
    notify_listeners('connected');
  }
  
  function on_notify(data) {
    if(verbose_) {
      logger_("Event:"+data.event); 
    }    
    notify_listeners(data.event, data.params);
  }
  
  function on_disconnect() {
    connected_ = false;
    logger_(type_str + ' disconnected.');
    notify_listeners('disconnected');
    if(reconnect_) {
    	setTimeout(function() {
    		_t.Connect(host_, port_, type_str);
    	},500);
    }
  }
  
  
  
  /*
   * Public functions
   */
  
  this.SendV3 = function(type, action, payload) {
    var header = type+":"+protocol_+":"+action+":";
    var bytes = [];
    for(var i = 0; i < header.length; ++i) {
      bytes.push(header[i]);
    }
    switch(protocol_) {
    case "msgpack":
      var bytes = msgpack.pack(payload)
      var total_size = header.length + bytes.length;
      var binary = new Uint8Array(total_size);
      
      for(i = 0; i < header.length; ++i) {
        binary[i] = header[i].charCodeAt();
      }
      for(i = header.length; i < total_size; ++i) {
        binary[i] = bytes[i-header.length];
      }
      socket_.send(binary.buffer);
      break;
    case "json":
      socket_.send(header+JSON.stringify(payload));
    }
  }

  this.Connect = function(host, port, typename) {
    host_ = host;
    port_ = port;
    protocol_ = "msgpack";
    _t = this;
    type_str = typename;
   
    
    if(verbose_) { logger_("Attempting to connect via "+typename) };
    socket_ = new WebSocket("ws://" + host_ + ":" + port_ + "/" + typename);
    socket_.binaryType = 'arraybuffer';
    socket_.onopen = function(event) {
    	on_connected();
    };
    socket_.onclose = function(event) {
    	on_disconnect();
    }
    
    socket_.onmessage = function(event) {
    	var data = new Uint8Array(event.data);
    	var delim = [];
    	var i = 0;
    	while(delim.length != 3) {
    		if(data[i] == 58) { // Test for semicolons
    			delim.push(i);
    		}
    		++i;
    	}
    	
    	// var msg = String.fromCharCode.apply(null, data);
    	
    	var type = String.fromCharCode.apply(null,data.subarray(0,delim[0]));
    	var protocol = String.fromCharCode.apply(null,data.subarray(delim[0]+1,delim[1]));
    	var event = String.fromCharCode.apply(null,data.subarray(delim[1]+1,delim[2]));
    	var payload = {};
    	switch(protocol) {
    	case 'msgpack':
    		reply = data.subarray(delim[2]+1,data.length)
    		payload = msgpack.unpack(reply)
    		break;
    	case 'string':
    		payload = String.fromCharCode.apply(null,data.subarray(delim[2]+1,data.length));
    		break;
    	case 'json':
    		// This is bugged, see
        // https://code.google.com/p/google-web-toolkit/issues/detail?id=7651
    		// payload =
        // JSON.parse(String.fromCharCode.apply(null,data.subarray(delim[1]+1,data.length)));
    		var reply = "";
    		for(var i = delim[2]+1; i < data.length; ++i) {
    			reply += String.fromCharCode(data[i]);
    		}
    		//
    		payload = JSON.parse(reply);
    		break;
    	}

    	notify_listeners(type, event, payload);    	    	
    }
  }
  
  this.Close = function() {
	  socket_.close();
  }
  
  this.Register = function(event, callback) {
    if(listeners_[event] == null) {
      listeners_[event] = new Array();
    }
    listeners_[event].push(callback);
  }
  
  this.UnRegister = function(event, callback) {
    if(listeners_[event] == null) {
      return;
    }
    var index = listeners_[event].indexOf(callback);
    if(index != -1) listeners_[event].splice(index,1); 
  }
    
  // Accessor functions
  this.is_connected = function() {
    return connected_;
  }
  this.version = function() {
    return version_;
  }
  // Mutator functions
  this.set_logger = function(new_logger) {
    logger_ = new_logger;
  }
  this.set_verbose = function(value) {
    verbose_ = value;
  }
  this.set_reconnect = function(value) {
    reconnect_ = value;
  }
  
  this.set_protocol = function(value) {
  	if(value == "json" || value == "msgpack") {
  		protocol_ = value;
  	}
  }
}