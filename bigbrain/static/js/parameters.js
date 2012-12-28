(function() {
  
  $('body').on('click','#add-parameter', function(e) {
    $("#add-parameter-modal").modal({
      backdrop:false
    });
  });
  
  $('body').on('click', '#save-new-parameter' ,function(e) {
    $("#add-parameter-modal").modal('toggle');
    var name = $("#new-param-name").val();
    exp.AddNewParameter(name);
  });
  
  exp.ListParams();
}());