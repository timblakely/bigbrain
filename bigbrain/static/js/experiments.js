$(document).ready(function() {
  $('#createnew').click(function() {
    $("#add-experiment-modal").modal({
      backdrop:false
    });
    
    $('body').on('click', '#save-new-experiment' ,function(e) {
      var name = $("#new-experiment-name").val();
      var description = $("#new-experiment-description").val();
      
      $.ajax({
        url:  '/experiments/ajax/addnew',
        type: "POST",
        data: JSON.stringify({
            name: name,
            description: description 
          }),
        success: function(reply) {
          window.location.href = '/experiments/';          
        },
        error: function(reply) {
          noty({
            text: 'Experiment could not be added',
            type: 'error'
          })
        }
      });
    });
  });
});