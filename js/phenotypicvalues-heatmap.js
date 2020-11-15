(function ($) {
  Drupal.behaviors.exampleModule = {
    attach: function (context, settings) {

      // First remove the old chart.
      d3.select("#phenotypicvalues-heatmap svg").remove();

      // set the dimensions and margins of the graph
      var margin = {top: 0, right: 10, bottom: 40, left: 30},
        width = 800 - margin.left - margin.right,
        height = 70 - margin.top - margin.bottom;

      var trait_id = Drupal.settings.divseeksearch.trait_id;
      if (typeof(trait_id) == 'number') {
        // append the svg object to the body of the page
        var svg = d3.select("#phenotypicvalues-heatmap")
        .append("svg")
          .attr("width", width + margin.left + margin.right)
          .attr("height", height + margin.top + margin.bottom)
        .append("g")
          .attr("transform",
                "translate(" + margin.left + "," + margin.top + ")");

        //Read the data
        var url = Drupal.settings.divseeksearch.dataurl + '/' + trait_id;
        d3.json(url , function(data) {

          // Labels of row and columns -> unique identifier of the column called 'group' and 'variable'
          var myGroups = d3.map(data, function(d){return d.group;}).keys()
          var myVars = d3.map(data, function(d){return d.variable;}).keys()
          var maxGerm = d3.max(data, function(d) { return d.value;} );

          // Build X scales and axis:
          var x = d3.scaleBand()
            .range([ 0, width ])
            .domain(myGroups)
            .padding(0.01);
          svg.append("g")
            .style("font-size", 15)
            .attr("transform", "translate(0," + height + ")")
            .call(d3.axisBottom(x).tickSize(0))
            .select(".domain").remove();

          // Build Y scales and axis:
          var y = d3.scaleBand()
            .range([ height, 0 ])
            .domain(myVars)
            .padding(0.05);

          // Build color scale
          var myColor = d3.scaleSequential()
            .interpolator(d3.interpolateBlues)
            .domain([1,maxGerm])

          // add the squares
          svg.selectAll()
            .data(data, function(d) {return d.group+':'+d.variable;})
            .enter()
            .append("rect")
              .attr("x", function(d) { return x(d.group) })
              .attr("y", function(d) { return y(d.variable) })
              .attr("width", x.bandwidth() )
              .attr("height", y.bandwidth() )
              .style("fill", function(d) { return myColor(d.value)} )
              .style("stroke-width", 4)
              .style("stroke", "none")
              .style("opacity", 0.8)
        })
      }
    }
  };
}(jQuery));
