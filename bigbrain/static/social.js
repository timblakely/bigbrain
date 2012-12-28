function Social() {
	
	var temporary_registers = [];
	
	var commstream = new Connection();
	commstream.set_reconnect(true);
	
	commstream.Connect(window.location.hostname,window.location.port,'commstream');
	
	function made_connection(event) {
		noty({
            text: 'Connected to BigBrain',
            timeout: 3000,
            type: 'success',
            layout: 'topRight'
        });
		commstream.Register('disconnected', lost_connection);
	}
	
	function lost_connection(event) {
		noty({
            text: 'Lost connection to BigBrain servers, attempting to reconnect...',
            timeout: 3000,
            type: 'error',
            layout: 'topRight'
        });
		commstream.UnRegister('disconnected', lost_connection);
	}
	
	commstream.Register('connected', made_connection);
	
	function notification(event, message) {
		noty({
            text: '<B><i>'+event+'</i></B><BR>'+message,
            timeout: 3000,
            type: 'alert',
            layout: 'topRight'
        });
	} 
	
	function error(event) {
		noty({
            text: 'Error:<BR>' + event.error,
            timeout: 3000,
            type: 'error',
            layout: 'topRight'
        });
	} 
	
	function shared_resource(event) {
		noty({
            text: event.from + ' shared an ' + event.type+ ' with you: ' + event.name,
            type: 'alert',
            layout: 'topRight',
            buttons: [
                {
                	addClass: 'btn btn-primary', 
                	text:'Ok',
                	onClick: function(thisnoty) {
                		thisnoty.close();
                	}
                }
            ]
        });
	}
	
	function instance_update(event) {
		noty({
            text: event.instance + ': ' + event.message,
            timeout: 3000,
            type: 'information',
            layout: 'topRight',
        });
	}
	
	function instance_error(event) {
		noty({
            text: event.instance + ': ' + event.message,
            timeout: 3000,
            type: 'error',
            layout: 'topRight',
        });
	}
	
	this.TemporarilyRegister = function(event, callback) {
		temporary_registers.push([event, callback]);
		commstream.Register(event, callback);
	}
	
	this.ClearTemporaries = function() {
		temporary_registers.forEach(function(tuple) {
			commstream.UnRegister(tuple[0], tuple[1]);
		});
		temporary_registers = [];
	}
	
	// Global
	commstream.Register('Notification', notification);
	commstream.Register('Error', error);
	//commstream.Register('UserLoggedIn', user_logged_in);
	//commstream.Register('SharedResource', shared_resource);
	//commstream.Register('InstanceUpdate', instance_update);
	//commstream.Register('InstanceError', instance_error);
}