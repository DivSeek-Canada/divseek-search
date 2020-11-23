(function ($) {
  Drupal.behaviors.divseekSearchAccessionHapmap = {
    attach: function (context, settings) {

      // First remove the old chart.
      d3.select("#phenotypicvalues-heatmap svg").remove();

      // set the dimensions and margins of the graph
      var margin = {top: 20, right: 250, bottom: 10, left: 30},
        width = 900 - margin.left - margin.right,
        height = 60 - margin.top - margin.bottom;

      Drupal.settings.divseeksearch.data.forEach(function(details) {
        var trait_id = details.trait_id;
        if (typeof(trait_id) == 'number') {

          details.unit_ids.forEach(function(unit_id) {

            var unit_name = details.units[unit_id];
            var id = "#phenotypicvalues-heatmap-" + trait_id + '-' + unit_id;

            d3.select(id + " svg").remove();

            // append the svg object to the body of the page
            var svg = d3.select(id)
            .append("svg")
              .attr("width", width + margin.left + margin.right)
              .attr("height", height + margin.top + margin.bottom)
            .append("g")
              .attr("transform",
                    "translate(" + margin.left + "," + margin.top + ")");

            //Read the data
            var url = Drupal.settings.divseeksearch.dataurl + '/' + trait_id + '/' + unit_id;
            d3.json(url , function(data) {

              // Labels of row and columns -> unique identifier of the column called 'group' and 'variable'
              var myGroups = d3.map(data, function(d){return d.group;}).keys()
              var myVars = d3.map(data, function(d){return d.variable;}).keys()
              var maxGerm = d3.max(data, function(d) { return d.value;} );

              // Build Y scales and axis:
              var y = d3.scaleBand()
                .range([ height, 0 ])
                .domain(myVars)
                .padding(0.05);

              // Build color scale
              var myColor = d3.scaleSequential()
                .interpolator(d3['interpolateBlues'])
                .domain([0, maxGerm]);
              console.log(myColor);

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

              // Labels for the unit, germplasm axis and value axis.
              var xlabel = svg.append("g")
                .attr("transform", "translate(" + (width + 10) + "," + y.bandwidth()/2 + ")");
              xlabel.append('text')
                .style("font-size", "14px")
                .style("text-decoration", "underline")
                .style("font-weight", "bold")
                .attr('y', 5)
                .attr('alignment-baseline', 'center')
                .style("fill", '#4D4D4D')
                .text(unit_name);
              var xlabel2 = svg.append("g")
                .attr("transform", "translate(" + (width + 10) + ",0)");
              xlabel2.append('text')
                .style("font-size", "10px")
                .attr('y', -5)
                .attr('alignment-baseline', 'center')
                .style("fill", '#4D4D4D')
                .text("Number of Germplasm");
              var xlabel3 = svg.append("g")
                .attr("transform", "translate(" + (width + 10) + "," + (y.bandwidth() + margin.top) + ")");
              xlabel3.append('text')
                .style("font-size", "10px")
                .attr('y', -8)
                .attr('alignment-baseline', 'center')
                .style("fill", '#4D4D4D')
                .text("Mean Value per Trait");

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

              // add labels for the squares
              svg.selectAll()
                .data(data, function(d) {return d.group+':'+d.variable;})
                .enter()
                .append("g")
                  .style("font-size", "10px")
                  .attr("transform", function(d) {
                    return "translate("+ x(d.group) +","+ y(d.variable) +")"})
                  .append('text')
                    .attr('y', -5)
                    .attr('x', x.bandwidth()/2)
                    .attr('text-anchor', 'middle')
                    .style("fill", '#000')
                    .text(function (d) { return d.value; });
            }) // END OF `d3.json(url , function(data) {`
          }) // END OF `data.unit_ids.forEach(function(unit_id) {`
        } // END OF `if (typeof(trait_id) == 'number') {`
      }) // END OF `Drupal.settings.divseeksearch.data.forEach(function(data) {`
    } // END OF `attach: function (context, settings) {`
  }; // END OF `Drupal.behaviors.divseekSearchAccessionHapmap = {`
}(jQuery)); // END OF `(function ($) {`
