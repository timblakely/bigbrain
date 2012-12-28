(function() {
  var instanceusage = bb.chart.create('#instanceusage', {
    axes: 'both',
  });
  instanceusage.addseries([0, 0, 100, 128, 200, 0, 20, 30, 0, 0, 0, 0, 60, 50, 70, 90, 0, 10, 12, 100, 180, 186, 190, 10, 0, 0, 0, 0],
    {marker: {style: {fill:'white'}}});
  instanceusage.render();
  
  var runsperday = bb.chart.create('#runsperday', {
    axes: 'both',
  });
  runsperday.addseries([0,0,2,3,0,4,0,3,5,4,0,0,5,1,0,3,0,3,5,0,0,0,0,0,8,9,0],
    {marker: {style: {fill:'white'}}});
  runsperday.render();
})();