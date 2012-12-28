function Renderer() {
	var stats;
	var renderer;
	var material;
	var uniforms;
	var attributes;
	var container;
	var camera;
	var geometry;
	var rendering_loop_enabled;
	var neuron_particles;
	var controls;
	
	var connection_lines = [];
	
	var has_network = false;
	
	var window_center;
	
	var viewstream;
	
	var self;

	var projector;
	var ray;
	var highlighted_neuron = null;
	var selected_neuron = null;
	var mouse = { x: 0, y: 0 };
	var check_intersection = true;
	var is_drag = false;
	
	var select_callback = null;
	
	var uuid_list = [];
	
	
	if (document.images) {
	    img1 = new Image();
	    img1.src = "/static/error.png";
	}
	
	this.UpdatePotentials = function(potentials) {
		if(!has_network) {
			return;
		}
		attributes.v.value = [];
		
		var mem_potentials = attributes.v.value;
		for (var i in potentials) {
			mem_potentials.push(potentials[i]); 
		}  
		attributes.v.needsUpdate = true;
	}

	this.RenderNetwork = function(neurons) {
		if(self.IsRendering()) {
			self.Stop();
		}
		geometry = new THREE.Geometry();
		  
		geometry.vertices = []  	
		
		attributes = {
			v: { type: 'f', value: [] }, 
			state: { type: 'i', value: [] } 
		};
	  
		var mem_potentials = attributes.v.value;
		var states = attributes.state.value;
		var vertex;
		uuid_list = [];
		if(typeof(neurons) != "string") {
			for (var i in neurons) {
				uuid_list.push(neurons[i].uuid);
				vertex = new THREE.Vector3();
			    vertex.x = neurons[i].x;
			    vertex.y = neurons[i].y;
			    vertex.z = neurons[i].z;// + Math.random()*10;
			    
			    vertex.multiplyScalar(10);
			    geometry.vertices.push(vertex);
			    
			    mem_potentials.push(-60);
			    states.push(0);
			}
		}
  
		uniforms = {
			size: { type: 'f', value: 30 },
			color_a: { type: 'c', value: new THREE.Color(0x220088) },
			color_b: { type: 'c', value: new THREE.Color(0x9999AA) },
			color_c: { type: 'c', value: new THREE.Color(0xAAAAAA) },
			color_d: { type: 'c', value: new THREE.Color(0xFF0000) },
			membrane_limits: { type: 'v4', value: new THREE.Vector4(-90.0, -60.0, -40.0, 30.0) },
			texture:   { type: "t", value: THREE.ImageUtils.loadTexture( "/static/spark1.png" ) }
		};
  
		material = new THREE.ShaderMaterial( {
			uniforms: 		uniforms,
			attributes:     attributes,
			vertexShader:   document.getElementById( 'vertexshader' ).textContent,
			fragmentShader: document.getElementById( 'fragmentshader' ).textContent,

			blending: 		THREE.NormalBlending,
			depthTest: 		false,
			transparent:	true
		});
  
		material.dynamic = true;
		material.needsUpdate = true;
  
		attributes.v.dynamic = true;
		attributes.v.needsUpdate = true;
		neuron_particles = new THREE.ParticleSystem(geometry, material);

		scene.add(neuron_particles);
		
		rendering_loop_enabled = true;
		
		has_network = true;
		render();
	}
	
	function update_intersection() {
		if(attributes === undefined) {
			return;
		}
		
		// Handle mouse selection
		var vector = new THREE.Vector3( mouse.x, mouse.y, 0.5 );
		projector.unprojectVector( vector, camera );

		ray = new THREE.Ray(camera.position , vector.subSelf( camera.position ).normalize());
				
		var intersects = ray.intersectObjects( [neuron_particles] );
		
		var states = attributes.state.value;
		
		if ( intersects.length > 0 ) {
			if(highlighted_neuron != intersects[ 0 ].vertex) {
				// different highlighted neuron than one selected
				if(highlighted_neuron != selected_neuron) {
					states[ highlighted_neuron ] = 0;	
				}
			}
			highlighted_neuron = intersects[ 0 ].vertex;
			states[ highlighted_neuron ] = 1;
			attributes.state.needsUpdate = true;
		}
		else {
			if(highlighted_neuron != selected_neuron) {
				states[ highlighted_neuron ] = 0;
				attributes.state.needsUpdate = true;
			}
			highlighted_neuron = null;
		}
		
	}
	
	function render() {
		if(rendering_loop_enabled) {
			requestAnimationFrame(render);
			if(check_intersection) {
				update_intersection();
				check_intersection = false;
			}
			controls.update();
			renderer.render(scene,camera);
			stats.update();
		}
	};

	function on_resize() {
		camera.aspect = container.width() / container.height();
		camera.updateProjectionMatrix();
		
		renderer.setSize(container.width(), container.height());
	}
	
	function mouse_moved(event) {
		//if(!is_drag) {
			check_intersection = true;
		//}
		mouse.x = (event.offsetX /container.innerWidth())*2-1;
		mouse.y = -(event.offsetY /container.innerHeight())*2+1
	}
	
	this.IsRendering = function() {
		return rendering_loop_enabled;
	}
	
	this.Stop = function() {
		scene.remove(neuron_particles);
		scene.remove(camera);
		self.ClearSynapses();
		
		rendering_loop_enabled = false;
		if(neuron_particles != null) {
			renderer.deallocateObject( neuron_particles );
		}
		if(material != null) {
			renderer.deallocateMaterial( material );	
		}
	  	if(geometry != null) {
	  		renderer.deallocateObject( geometry );	
	  	}
	  	if(controls != null) {
	  		renderer.deallocateObject(controls);
	  	}
	}
	
	this.SelectedCallback = function(callback) {
		select_callback = callback;
	}
	
	this.Display = function(divname, connection) {
		
		self = this;
		viewstream = connection;
		
		container = $(divname);
		if(container.count == 0) {
			console.log("Container '" + divname + "' for webgl doesn't exist!")
			return;
		}

		// Set up renderer
		try {
			renderer = new THREE.WebGLRenderer( { clearColor: 0xFFFFFF, clearAlpha: 1 } );
		}
		catch(err) {
			noty({
	            text: 'Error: Couldn\'t start WebGL! Try restarting browser',
	            timeout: 3000,
	            type: 'error',
	            layout: 'topRight'
	        });
			container.append("No WebGL, try restarting.  See console for details");
			console.log("Error: ",err.name);
			console.log(err.message);
			console.log(err.stack);
			return;
		}
		renderer.setSize(container.width(), container.height());
		renderer.domElement.style.position = "absolute";
		renderer.domElement.style.left = "0px";
		
		container.append(renderer.domElement);
		window.addEventListener('resize',on_resize, false);
		
		var canvas = $(renderer.domElement);
		
		canvas.mousemove(mouse_moved);
		
		canvas.on('mousedown',function() {
			is_drag = false;
		})
		canvas.on('mousemove',function() {
			is_drag = true;
		})
		
		canvas.click(function(event) {
			if(is_drag) {
				is_drag = false;
				return;
			}
			if(highlighted_neuron != null) {
				if(selected_neuron != null) {
					attributes.state.value[selected_neuron] = 0;
				}
				selected_neuron = highlighted_neuron;
				attributes.state.value[selected_neuron] = 1;
				attributes.state.needsUpdate = true;
				if(select_callback) {
					select_callback(uuid_list[selected_neuron]);
				}
				else {
					console.log('Would call select_callback with uuid: ' + uuid_list[SELECTED_NEURON]);
				}
			} 
			else {
				if(selected_neuron != null) {
					attributes.state.value[selected_neuron] = 0;
				}
				attributes.state.needsUpdate = true;
				if(select_callback) {
					select_callback(null);
				}
				else {
					console.log('Unselected!');
				}
			}
		});
		
		// New Scene
		scene = new THREE.Scene();
		
		// New Camera
		camera = new THREE.PerspectiveCamera(45, container.width()/container.height(), 1, 10000);
		scene.add(camera);
		
		// Set up camera for projecting ray
		projector = new THREE.Projector();
		ray = new THREE.Ray();
		
		// Move camera to viewable position
		camera.position.z = 400;
		controls = new THREE.OrbitControls( camera, renderer.domElement );
		
		rendering_loop_enabled = true;	
		
		// Set up stats
		stats = new Stats();
		stats.domElement.style.float = 'left';
		stats.domElement.style.top = '0px';
		container.append( stats.domElement );
		
		render();
	}
	
	this.AddSynapses = function(source, targets) {
		var from_idx = uuid_list.indexOf(source);
		var to_idx = _.map(targets, function(d) { return uuid_list.indexOf(d); });
		
		_.each(to_idx, function(to) {
			var x,y,z;
			do {
				x = 2 * Math.random() - 1;
				y = 2 * Math.random() - 1;
				z = 2 * Math.random() - 1;
			} while(x * x + y * y + z * z< 1);
			
			var rand_vec = new THREE.Vector3(x,y,z);
			rand_vec.normalize();
			var from_vert = neuron_particles.geometry.vertices[from_idx];
			var to_vert = neuron_particles.geometry.vertices[to];
			var vec = new THREE.Vector3();
			vec.sub(to_vert,from_vert);
			var len = vec.length();
			var half_vec = vec.divideScalar(4);
			var final_rand_vec = new THREE.Vector3().add(half_vec,rand_vec.multiplyScalar(len/10));
			
			var first_vert = new THREE.Vector3().add(from_vert,final_rand_vec);
			var second_vert = new THREE.Vector3().add(to_vert,final_rand_vec.multiplyScalar(-1));
			
			var spline = new THREE.Spline([from_vert,first_vert, second_vert,to_vert]);
			
			var n_sub = 6;
			var geo = new THREE.Geometry();
			var colors = [];
			
			for(i = 0; i < 4 * n_sub; ++i) {
				idx = i / (4 * n_sub);
				pos = spline.getPoint(idx);
				geo.vertices[i] = new THREE.Vector3(pos.x, pos.y, pos.z);
				geo.colors[i] = new THREE.Color(0x00FF00);
			}
			
			material = new THREE.LineBasicMaterial({ 
				color: 0xffffff, 
				opacity: 0.5, 
				linewidth: 3, 
				vertexColors: THREE.VertexColors 
			});
			
			line = new THREE.Line(geo, material);
			scene.add(line);
			connection_lines.push(line);
		});
	}
	
	this.ClearSynapses = function() {
		_.each(connection_lines, function(line) {
			scene.remove(line);
		});
		connection_lines = [];
	}
}