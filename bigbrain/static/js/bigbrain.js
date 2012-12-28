$.fn.exists = function() {
  return this.length !== 0;
}

Array.prototype.max = function() {
  return Math.max.apply(null, this);
}
Array.prototype.min = function() {
  return Math.min.apply(null, this);
}

var bb = (function() {
  bb = {
    version: "0.0.1",
    ajax_spinner: null
  };
  
  var bbChart = {
          
          // DATASET
          dataset: {
            
            // Needs to be a closure
            defaults: function(_t) {
              defaults = {
                marker: {
                  shape: 'circle',
                  size: 2,
                  style: {
                    fill: 'cyan',
                    stroke: 'black',
                    'stroke-width': 1
                  },
                  cx: function(d,e) { 
                    return _t.chart.xscale(e) 
                  },
                  cy: function(d) {
                    return _t.chart.yscale(d)
                  }
                },
                line: {
                  x: function(d,e) {
                    return _t.chart.xscale(e)
                  },
                  y: function(d,e) {
                    return _t.chart.yscale(d)
                  },                
                  style: {
                    stroke: 'blue',
                    'stroke-width': 1,
                  }
                },
                name: function() {
                  return 'dataset_' + _t.datasets_ref.length;
                }
              };
              return defaults;
            },
            
            init: function(chart, options) {
              this.chart = chart;
              var _t = this;
              this.opt = $.extend(true, {}, this.defaults(_t), options);
              this.datasets_ref = this.chart.svg.selectAll('g.datasets');
              this.g = this.datasets_ref.append('g')
                .attr('id',this.opt.name);
              this.graphics = {};
              return this;
            },
            
            setdata: function(data) {
              this.data = data;
            },
            
            render: function() {
              if(this.opt.line !== 'none') {                
                var line = d3.svg.line()
                  .interpolate('monotone')
                  .x(this.opt.line.x)
                  .y(this.opt.line.y);
                
                var paths = this.g.selectAll('path')
                  .data([this.data]);
                paths.enter()
                  .append('path');
                paths
                  .transition()
                    .ease('cubic-in-out')
                    .duration(250)
                    .attr('d',line);
                
                for(s in this.opt.line.style) {
                  paths
                    .attr(s,this.opt.line.style[s]);
                }
              }
              if(this.opt.marker !== 'none') {
                var pts = this.g.selectAll(this.opt.marker.shape)
                  .data(this.data);
                var newmarkers = pts.enter()
                  .append(this.opt.marker.shape)
                  .attr('r', this.opt.marker.size)
                  .attr('cx', this.opt.marker.cx)
                  .attr('cy', this.opt.marker.cy);
                if(!newmarkers.empty()) {
                  for(s in this.opt.marker.style) {
                    newmarkers
                      .attr(s,this.opt.marker.style[s]);
                  }
                }
                pts
                  .transition()
                    .duration(250)
                    .ease('cubic-in-out')
                    .attr('cx', this.opt.marker.cx)
                    .attr('cy', this.opt.marker.cy)
                pts.exit()
                  .remove();
              }
            }
          },
  
          // CHART
  
          defaults: {
            axes: 'both',
            color: '#FFF',
            margin: 'auto',
            label: 'auto',
            scaling: 'auto',
            scale: {
              x: [0, 1],
              y: [0, 1],
            }
          },
          
          init: function(options) {
            var _t = this;
            
            this.container = options.container;
            delete options.container;
            if(options.scale) {
              options.scaling = 'manual';
            }
                        
            this.opt = $.extend(true, {}, this.defaults, options);
            
            // Axes
            if(this.opt.axes != 'none') {
              // Auto margins
              if(this.opt.margin == 'auto') {
                this.opt.margin = {
                        top: 0,
                        right: 0,
                        bottom: 0,
                        left: 0
                } 
                
                if(this.opt.axes == 'both' || this.opt.axes == 'x') {
                  this.opt.margin.bottom = 20;
                  this.opt.margin.right = 5;
                  if(this.opt.axes == 'x') {
                    this.opt.margin.left = 5;
                  }
                }
                if(this.opt.axes == 'both' || this.opt.axes == 'y') {
                  this.opt.margin.left = 40;
                  this.opt.margin.top = 10;
                  if(this.opt.axes == 'y') {
                    this.opt.margin.bottom = 10;
                  }
                }
              }
            }
            
            this.series = [];
            
            this._build();
            return this;
          },
          
          addseries: function(data, options) {
            
            var dataset = Object.create(this.dataset).init(this, options);
            dataset.setdata(data);
            this.series.push(dataset);
            return dataset;
          },
          
          render: function() {
            // If the chart is AUTO scaled, rescale it
            if(this.opt.scaling == 'auto') {
              this._rescale();
            }
            for(var i = 0; i < this.series.length; ++i) {
              this.series[i].render();
            }
          },          
          
          _build: function() {
            var t = this.opt.margin.top;
            var r = this.opt.margin.right;
            var l = this.opt.margin.left;
            var b = this.opt.margin.bottom;
            
            var w = this.container.width();
            var h = this.container.height();
            
            var svg = d3.select(this.container[0])
              .append('svg')
              .style('background',this.opt.color);
            this.svg = svg;
            
            svg.attr('width',w);
            svg.attr('height',h);
            
            if(this.opt.axes == 'both' || this.opt.axes == 'x') {
              this.xscale = d3.scale.linear()
                .domain(this.opt.scale.x)
                .range([l, w - r]);
              this.xaxes = d3.svg.axis()
                .scale(this.xscale)
                .ticks(5)
                .orient('bottom');
              svg.append('g')
                .attr('class','x axis')
                .attr('transform','translate(0,'+(h-b)+')')
                .call(this.xaxes);
            }
            if(this.opt.axes == 'both' || this.opt.axes == 'y') {
              this.yscale = d3.scale.linear()
                .domain(this.opt.scale.y)
                .range([h-b, t]);
              this.yaxes = d3.svg.axis()
                .scale(this.yscale)
                .ticks(5)
                .orient('left');
              svg.append('g')
                .attr('class','y axis')
                .attr('transform','translate('+l+',0)')
                .call(this.yaxes);
            }
            svg.append('g')
              .attr('class','datasets');
          },
          
          _rescale: function() {
            // Scale            
            var x = [0, 0], y = [0, 0];
            if(this.opt.scaling == 'auto') {
              if(this.series.length == 0) {
                return;
              }
              x[0] = 0;
              x[1] = -Infinity;
              for(s=0; s < this.series.length; ++s) {
                x[1] = Math.max(x[1],this.series[s].data.length)
              }
              
              y[1] = -Infinity;
              for(s=0; s < this.series.length; ++s) {
                y[1] = Math.max(y[1],this.series[s].data.max());
              }
              y[0] = Infinity;
              for(s=0; s < this.series.length; ++s) {
                y[0] = Math.min(y[0],this.series[s].data.min());
              }
            }
            else {
              x = this.opt.scale.x;
              y = this.opt.scale.y;
            }
            
            this.xscale.domain(x);
            this.yscale.domain(y);
            this.svg.selectAll('.x.axis')
              .transition()
                .duration(250)
                .ease('cubic-in-out')
                .call(this.xaxes);
            this.svg.selectAll('.y.axis')
              .transition()
                .duration(250)
                .ease('cubic-in-out')
                .call(this.yaxes);
          }
            
            
  }
     
  bb.chart = {}
  bb.chart.create = function(container, options) {
    var where = $(container);
    if(!where.exists()) {
      console.log('Container for chart doesn\'t exist');
      return;
    }
    
    options = $.extend({}, options, {container: where});
    
    var chart = Object.create(bbChart).init(options);
    return chart;
  }  
  
  bb.Experiment = {
          parameter: {
            defaults: {
              min: 0,
              max: 1,
              step: 0.1,
              enabled: true,
              offvalue: 1
            },
            template: {
              list: '<li> \
                <a href="#{{name}}">{{name}}</a> \
                </li>',
              details: ' \
                <div class="param-holder" id="{{name}}"> \
                <div class="well well-small param-box">\
                  <div class="row-fluid">\
                    <div class="span4 param-label">\
                      <button class="close pull-right">&times;</button>\
                      <div class="param-label-content">\
                      {{name}}\
                      </div>\
                    </div>\
                    <div class="span4 options">\
                      <div class="row-fluid">\
                        <label class="span4 control-label" id="enabled">Enabled:</label>\
                        <div class="btn-group">\
                          <button class="btn btn-mini enable {{__enabled}}">On</button>\
                          <button class="btn btn-mini disable {{__disabled}}">Off</button>\
                        </div>\
                      </div>\
                      <div class="row-fluid">\
                        <label class="span4 control-label" id="type">Type:</label>\
                        <div class="span6 input-append input-medium dropdown combobox">\
                          <input class="bb-thin" type="text" value="Range">\
                          <button class="btn btn-mini bb-combo" data-toggle="dropdown"><i class="caret"></i></button>\
                          <ul class="dropdown-menu">\
                            <li><a href="#">Range</a></li>\
                            <li><a href="#">Option</a></li>\
                          </ul>\
                        </div>\
                      </div>\
                      <div class="row-fluid">\
                        <label class="span4 control-label" id="type">Off Value:</label>\
                        <input class="bb-thin" type="text" name="offvalue" value="{{offvalue}}">\
                      </div>\
                    </div>\
                    <div class="span8 editor">\
                      <div class="row-fluid">\
                      <label class="span6 control-label">Start</label>\
                      <input class="span6 bb-thin" type="text" name="min" value="{{min}}">\
                      </div>\
                      <div class="row-fluid">\
                      <label class="span6 control-label" id="step">Step</label>\
                      <input class="span6 bb-thin" type="text" name="step" value="{{step}}">\
                      </div>\
                      <div class="row-fluid">\
                      <label class="span6 control-label" id="stop">Stop</label>\
                      <input class="span6 bb-thin" type="text" name="max" value="{{max}}">\
                      </div>\
                    </div>\
                  </div>\
                </div>\
              </div>'
            },
            
            Init: function(name, experiment, value) {
              this.val = $.extend(true, {}, this.defaults, value);
              this.val.name = name;
              this.experiment = experiment;
              this.val.__disabled = function() { if(!this.enabled) return 'btn-danger' }
              this.val.__enabled = function() { if(this.enabled) return 'btn-success' }
              this.elements = {};
              return this;
            },
            
            ListIn: function(container) {
              this.elements.list = $(Mustache.to_html(this.template.list,this.val))
                .appendTo(container);
              return this;
            },
            
            DetailsIn: function(container) {
              var _t = this;
              
              this.elements.details = $(Mustache.to_html(this.template.details,this.val))
                .appendTo(container);
              
              this.elements.details.on('click','.close',function(e) {
                $(this).popover({
                  delay: {show:250, hide:100},
                  trigger: 'manual',
                  title: 'Confirm',
                  placement: 'top',
                  html: true,
                  content: 'Delete this variable?<button class="btn btn-mini pull-right delete-confirm-no">No</button><button class="btn btn-danger pull-left delete-confirm-yes btn-mini">Yes</button>'
                });
                $(this).popover('show');
              });
              
              this.elements.details.on('click','.delete-confirm-yes',function(e) {
                var parent = $(this).closest(".param-holder");
                _t.experiment._get('parameters/delete', _t.val, {
                  success: function(val) {
                    _t.experiment.RemoveParameter(_t);
                    _t.elements.list.remove();
                    _t.elements.details.remove();
                  }
                })
              });
              
              this.elements.details.on('click','.delete-confirm-no',function(e) {
                $(this).closest('.popover').remove();
              });
              
              this.elements.details.on('click','.enable', function(e) {
                if($(this).hasClass('btn-success')) {
                  return;
                }
                var parent = $(this).closest(".param-holder");
                _t.val.enabled = true;
                var _btn = this;
                _t.experiment._get('parameters/update', _t.val, {
                  success: function(val) {
                    $(_btn).addClass('btn-success');
                    $(_btn).siblings().removeClass('btn-danger');
                  }
                });
              });
              
              this.elements.details.on('click','.disable', function(e) {
                if($(this).hasClass('btn-danger')) {
                  return;
                }
                var parent = $(this).closest(".param-holder");
                _t.val.enabled = false;
                var _btn = this;
                _t.experiment._get('parameters/update', _t.val, {
                  success: function(val) {
                    $(_btn).addClass('btn-danger');
                    $(_btn).siblings().removeClass('btn-success');
                  }
                });
              });
              
              this.elements.details.on('change','input', function(e) {
                // This cast could cause problems for non-numerical values
                _t.val[$(this).attr('name')] = parseFloat($(this).val());
                _t.experiment._get('parameters/update',_t.val);
              });
              
              return this;
            } 
          },
  
          Init: function(id) {
            this.id = id;
            var _exp = this;
            this.parameters = [];
            
            return this;
          },
          
          _get: function(url, payload, options) {
            var defaults = {
                    success: null,
                    error: null
            } 
            opts = $.extend(true, {}, defaults, options);
            var modifier = '?';
            if(url.indexOf('?') >= 0) {
              modifier = '&';
            }
            
            url.replace(/^\//g, '');
            
            url = '/experiments/ajax/' + url + modifier + 'id=' + this.id;
            
            $.ajax({
              url:  url,
              type: "POST",
              data: JSON.stringify(payload),
              success: opts.success,
              error: opts.failed
            });
          },
          
          _addparam: function(param) {
            this.parameters.push(param);
            param.ListIn("#param-list")
              .DetailsIn("#param-details");
            
            
          },
          
          _displayparams: function(params) {
            for(var i = 0; i < params.length; ++i) {
              this._addparam(params[i]);
            }
          },
          
          AddNewParameter: function(name) {
            var _exp = this;
            var p = Object.create(bb.Experiment.parameter).Init(name, this);
            this._get('parameters/add', p.val, {
              success: function(val) {
                _exp._addparam(p);
              }
            });
          },
          
          RemoveParameter: function(parameter) {
            this.parameters.splice(this.parameters.indexOf(parameter),1);
          },
          
          ListParams: function() {
            var _t = this;
            
            this._get('parameters/list', {}, {
              success: function(val) {
                params = JSON.parse(val);
                for(var i = 0; i < params.length; ++i) {
                  var p = Object.create(bb.Experiment.parameter).Init(params[i].name, _t, params[i]);
                  _t._addparam(p); 
                }
              }
            });
          }
  }
  
  bb.Experiment.Create = function(id) {
    var exp = Object.create(bb.Experiment).Init(id);
    return exp;
  }
  
  /*
   * 
   * Editor
   * 
   */
  bb.Editor = {
    version: "0.1",
    ajax_spinner: null,
    dom_element: null,
    save_button: null,
    
    _save: function() {
      
      var post_url = "/experiments/ajax/editor/save";
      
      if(this.ajax_spinner) {
        $(this.ajax_spinner)
            .html('<img src="/static/ajax.gif">')
      }
      
      $.ajax({
        url:  post_url + "?id=" +  exp.id,
        type: "POST",
        data: $(this.dom_element).val(),
        success: function(html) {
          noty({
            text: 'Saved!',
            timeout: 3000,
            type: 'success',
            layout: 'topRight'
          });
          $(this.ajax_spinner).html("");
        },
        error: function(html) {
          noty({
            text: 'Failed to save! Please be careful you do not lose any work!',
            timeout: 3000,
            type: 'error',
            layout: 'topRight'
          });
          $(this.ajax_spinner).html("");
        }
      });
    },
    
    Make: function(dom_element, save_button) {
      var _t = this;
      if(!dom_element) {
        return;
      }
      this.dom_element = dom_element;
      this.save_button = save_button;
      $(this.save_button).click(function(e) {
        _t._save();
      });
      
      $(this.dom_element).keydown(function(event) {
        //19 for Mac Command+S
        if (!( String.fromCharCode(event.which).toLowerCase() == 's' && 
                event.ctrlKey) && !(event.which == 19)) return true;
          $(_t.save_button).click();
          event.preventDefault();
        return false;
      });
    }
  };
  
  return bb;
}());