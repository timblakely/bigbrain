function StateGraph(title_, stream_) {
	var _t = this;
	var title = title_;
	var stream = stream_;
	var graph_container;
	var colors = [
	    [0,0,255],
	    [255,0,0],
	    [0,255,0],
	    [255,0,255],
	    [255,255,0],
	    [0,255,255],
	    [0,0,0]
	];
	
	var xscale;
	var yscale;
	var yaxis;
	var path;
	
	var data = []
	
	graph_container = $('<div id="stategraph"></div>').appendTo('.glview');
	graph_container.append('<h1>'+title+'</h1><a href="#">X</a>');
	graph_container.children('a').click(function(event) {
		_t.Close();
	});
	
	var margins = {
			top: 6,
			right: 0,
			bottom: 19,
			left: 20
	}
	
	var svg = d3.select('#stategraph')
		.append('svg');
	
	var width = graph_container.width()-margins.right-margins.left;
	var height = graph_container.height() - margins.top - margins.bottom;
	
	xscale = d3.scale.linear()
		.domain([0,400])
		.range([0,width]);
	
	yscale = d3.scale.linear()
		.domain([0,1])
		.range([height, 0]);
	
	var line = d3.svg.line()
		.interpolate('linear')
		.x(function(d,i) { return xscale(i); })
		.y(function(d,i) { return yscale(d); });
	
	svg.append("g")
	.attr("class", "x axis")
	.attr("transform", "translate("+(margins.left-1)+"," + (height + margins.top) + ")")
	.call(d3.svg.axis().scale(xscale).orient("bottom"));
	
	yaxis = d3.svg.axis().scale(yscale).ticks(5).orient('left')
	
	svg.append('g')
		.attr('class','y axis')
		.attr('transform','translate('+(margins.left-1)+','+margins.top+')')
		.call(yaxis);
	
	path = svg.append('g')
		.attr('transform','translate('+(margins.left)+','+margins.top+')')
	  .append('path')
	  	.data([data])
	  	.attr('class','line')
	  	.attr('d',line);
	
	this.AddTrace = function (state_name) {
		var color_div = $('td:contains("'+a+'")',states_table).siblings('.graphcolor');
		var color_idx = data.length; 
		color_div.css('background','rgb('+colors[color_idx][0]+','+colors[color_idx][1]+','+colors[color_idx][2]+')')
	}
	
	this.RemoveTrace = function (state_name) {
		if(data.length == 0) {
			_t.Close();
		}
	}
	
	this.ActivePlot = function() {
		return title;
	}
	
	this.AppendData = function(datum) {
		datum = parseFloat(datum);
		data.push(datum);
		if(data.length > 400) {
			data.shift();
		}
		svg.select('.line')
			.attr('d', line);
		yscale.domain([d3.min(data),d3.max(data)]).nice();
		svg.select('.y.axis').call(yaxis);
	}
	
	this.Close = function() {
		$('.graphicon').removeClass('selected');
		graph_container.remove();
	} 
}

function CellInspector(uuid_, stream_, instance_) {
	var _t = this;
	var inspector;
	var params;
	var param_table
	var states;
	var states_table;
	var uuid = uuid_;
	var stream = stream_;
	var graph = null;
	var instance = instance_;
	
	inspector = $('<div id="cellinspector"></div>').appendTo('.glview');
	inspector.append('<h1>Cell ID: '+uuid+'</h1><a href="#">X</a>');
	inspector.children('a').click(function(event) {
		_t.Close();
	});
	
	params = $('<div class="cellinfo"></div>').appendTo(inspector);
	params.append('<h2>Parameters</h2>');
	param_table = $('<div class="scrollabledata"><table class="datadisplay"></table></div>').appendTo(params).children('table');
	
	states = $('<div class="cellinfo"></div>').appendTo(inspector);
	states.append('<h2>States</h2>');
	states_table = $('<div class="scrollabledata"><table class="datadisplay"></table></div>').appendTo(states).children('table');	
	
	plot = function(param_name) {
		if(graph) {
			graph.Close();
		}
		graph = new StateGraph(param_name, stream);
	}
	
	update_state = function(state) {
		if(states_table.children().size() == 0) {
			state_map = {};
			for(a in state) {
				var new_row = '<tr><td class="tablelabel">'+a+'</td><td class="tablevalue"></td><td class="graphcolor"></td><td class="graphicon"></td></tr>';
				var jq_row = $(new_row);
				jq_row.appendTo(states_table);
				var tablevalue = jq_row.children('.tablevalue');
				var icon = jq_row.children('.graphicon')
				icon.click(
					(function(param_name) {
						return function(event) {
							plot(param_name);
							$('.graphicon.selected').removeClass('selected');
							$(this).addClass('selected');
						};
					})(a)
				);  				
			}
			for(a in state) {
				state_map[a] = $('td:contains("'+a+'")',states_table).siblings()[0]; 	
			}
		}
	    for(a in state) {
	    	if(graph) {
	    		if(graph.ActivePlot() == a) {
	    			graph.AppendData(state[a]);
	    		}
	    	}
	    	state_map[a].innerHTML = state[a]; 	
		} 	
	}
	
	update_parameters = function(parameters) {
		for(i in parameters) {
			param_table.append('<tr><td class="tablelabel">'+i+'</td><td class="tablevalue">'+parameters[i]+'</td></tr>');
		}
	}
	
	UpdateParameters = function(data) {
		update_parameters(data);
	}
	
	UpdateState = function(data) {
		update_state(data.state);
	} 
	
	this.Close = function() {
		inspector.remove();
		stream.StopAgent('NeuronState', instance);
    
    if(graph) {
    	graph.Close();
    }
	}
	
  stream.Query('CellParameters', {'uuid':uuid, 'instance':instance}, UpdateParameters);
  stream.StartAgent('NeuronState', instance, UpdateState);
  stream.AddUUID('NeuronState', instance, uuid);
  /*stream.Query('GetCellParameters',  {'uuid':uuid}, UpdateParameters);*/
}

function  Viewstream(instance_name) {
	var _t = this;
	var instance = instance_name;
	var current_uuid;
	var state_update;
	var state_map = {};
	var inspector = null;
	var _query_callbacks = {};
  var _agent_callbacks = {};
	
	/*  
	 * 
	 *     Streaming connection functions
	 * 
	 */
	
	stream = new Connection();
	stream.Connect(window.location.hostname,window.location.port,'viewstream');
	
	function instance_failed(data) {
       $('.glview canvas').remove();
       $('.glview #stats').remove();
       $('<div id="error"></div>').appendTo('.glview').animate({
           opacity: 1,
           marginTop: '-64px'
       },1000);     
       noty({
           text: 'Instance failed!',
           timeout: 3000,
           type: 'error',
           layout: 'topRight'
        });   
        stream.Close();
        renderer.Stop();
    }
	
	this._query_message = function (event, payload) {
    if(_query_callbacks[event]) {
      for (var i=0; i < _query_callbacks[event].length; i++) {
        _query_callbacks[event][i](payload);
      }
      // Only want to answer queries once!
      _query_callbacks[event] = [];
    }
  }
  
  this._agent_message = function (event, payload) {
    if(_agent_callbacks[event]) {
      for (var i=0; i < _agent_callbacks[event].length; i++) {
        _agent_callbacks[event][i](payload);
      }
    }
  }
	
  this.Query = function (name, params, callback) {
	  if(callback) {
      if(_query_callbacks[name] == null) {
        _query_callbacks[name] = new Array();
      }
      _query_callbacks[name].push(callback);
    }
	  stream.SendV3("Query", name, params);
	}
	
  this.StartAgent = function (type, params, callback) {
    if(callback) {
      if(_agent_callbacks[type] == null) {
        _agent_callbacks[type] = new Array();
      }
      _agent_callbacks[type].push(callback);
    }
    stream.SendV3('Agent','StartAgent',{type:type,instance:instance});
  }
  
  this.StopAgent = function (type, params) {
    stream.SendV3('Agent','StopAgent',{type:type,instance:instance});
    delete _agent_callbacks[type];
  }
  
  this.AddUUID = function(agent_type, instance, uuid) {
    stream.SendV3('Agent','AddUUIDsToAgent', 
      {type:agent_type,instance:instance,uuids:[uuid]});
  }
	
	function on_connected(data) {
       noty({
           text: 'Connected to ' + instance,
           timeout: 3000,
           type: 'information',
           layout: 'topRight'
        });
       
       _t.Query("CellLocations", {instance:instance}, function(reply) {
         renderer.RenderNetwork(reply);
         _t.StartAgent('MembranePotential', instance, function(data) {
           renderer.UpdatePotentials(data.v);
         });
       });
    }
	
	function disconnected(data) {
       $('.glview canvas').remove();
       $('.glview #stats').remove();
       $('<div id="error"></div>').appendTo('.glview').animate({
           opacity: 1,
           marginTop: '-64px'
       },1000);       
       noty({
           text: 'Disconnected from ' + instance,
           timeout: 3000,
           type: 'information',
           layout: 'topRight'
       });
       renderer.Stop();
	}
	
	stream.Register('connected', on_connected);          
	stream.Register('disconnected', disconnected);
	stream.Register('Query', this._query_message);
	stream.Register('Agent', this._agent_message);
	
	/*  
	 * 
	 *    Rendering
	 * 
	 */
	
	var renderer = new Renderer();
  renderer.Display('.glview', this);
  
  function draw_synapses(data) {
  	var source = data.source;
  	var targets = data.targets;
  	
  	renderer.AddSynapses(source, targets);
  }
  
  function selected_callback(uuid) {
  	if(inspector) {
  		inspector.Close();
  		renderer.ClearSynapses();
  	}
  	if(uuid == null) {
  		return;
  	}
  	
  	inspector = new CellInspector(uuid, _t, instance);    
  	current_uuid = uuid;
  	
  	_t.Query("CellConnections", {uuid:uuid, instance:instance}, draw_synapses);
  }
  
  renderer.SelectedCallback(selected_callback); 
  
  this.Close = function() {
  	stream.Close();
  }      
}