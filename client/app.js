var heatmap = null;

var giggleUrl            = "http://potter.genetics.utah.edu:8080/";
var giggleUCSCBrowserUrl = "http://potter.genetics.utah.edu:8081/"
var ucscBrowserUrl       = "https://genome.ucsc.edu/cgi-bin/hgTracks?db=hg19";

var sourceFileMap = {};	
var coordMap = {};

var ucscFileMap = {};
var ucscTrackNames = [];

$(document).ready(function() {
	$.material.init();

	loadUCSCDefinition();

	heatmap = new heatmapD3().cellSize(15)
                             .legendCellSize(20)
                             .margin({top: 10, bottom: 10, left: 10, right: 70})
                             .colors(colorbrewer.YlGnBu[9])
                             //.colors(colorbrewer.Oranges[9]);
                             .on('d3click', function(d,i) {
                             	loadOverlapDetail(d.name, d.row, d.col);
                             });

	loadHeatmap();
});

function loadUCSCDefinition() {

	var giggleTracksDefUrl = giggleUCSCBrowserUrl + "?data";


	$.ajax({
	    url: giggleTracksDefUrl,
	    type: "GET",
	    crossDomain: true,
	    dataType: "text",
	    success: function(data) {

	    	var def = JSON.parse(data);
			def.sourceFiles.forEach( function( sourceFile ) {
				var fileName = sourceFile.name.split("tracks/")[1];
				ucscFileMap[fileName] = sourceFile.position[0];
			});

			def.dimensions[0].elements.forEach( function(trackName) {
				ucscTrackNames.push(trackName);
			})
			

		},
	    error: function(error) {
	    	console.error;
	    }
	});	

}

function loadUCSCTracks(chr, start, end) {
	var giggleTracksUrl = giggleUCSCBrowserUrl + "?region=" + chr + ":" + start + '-' + end;
	$.ajax({
	    url: giggleTracksUrl,
	    type: "GET",
	    crossDomain: true,
	    dataType: "text",
	    success: function(data) {
	    	var records = [];
			data.split("\n").forEach(function(row) {
				if (row == null || row.trim() == "") {

				} else {
					fields = row.split("\t");
					
					var rec = {};
					rec.name     = fields[0].split("#tracks/")[1];
					rec.size     = fields[1];
					rec.overlaps = fields[2];

					var pos      = ucscFileMap[rec.name];
					rec.pos      = pos;
					rec.trackName = ucscTrackNames[+pos];
					records.push(rec);

				}
			});

			var ucscTracksUrl = ucscBrowserUrl + '&position=' + chr + ":" + start + '-' + end;
			records.forEach( function(record) {
				if (+record.overlaps > 0) {
					ucscTracksUrl += "&" + record.trackName + "=dense";
				}
			});
			var newTab = window.open(ucscTracksUrl, '_blank');
			//newTab.focus();




	    },
	    error: function(error) {
	    	console.log(error);
	    }
	});
}


function loadOverlapDetail(fileName, row, col) {
	var rowLabel = coordMap[row + '-' + col].rowLabel;
	var colLabel = coordMap[row + '-' + col].colLabel;
	var detailUrl = giggleUrl + "?region=" + $('#overlaps').val() + "&files=" + fileName + "&full";
	$.ajax({
	    url: detailUrl,
	    type: "GET",
	    crossDomain: true,
	    dataType: "text",
	    success: function(data) {
	    	var results = [];
	    	var header = null;
	    	var records = [];
			data.split("\n").forEach(function(row) {
				if (row == null || row.trim() == "") {

				} else {
					fields = row.split("\t");
					if (row.indexOf("#") == 0) {
						if (header) {
							results.push({'header': header, 'rows': records});
							recs = [];
						}
						header = {};
						header.name = fields[0].split("#split/")[1];
						header.size = fields[1];
						header.overlaps = fields[2];
					} else {
						var rec = {};
						rec.chr   = fields[0];
						rec.start = fields[1];
						rec.end   = fields[2];
						records.push(rec);
					}

				}
			});
			if (header) {				
				results.push({'header': header, 'rows': records});
			}
			$('#overlaps-modal .modal-header').html("<h5>" + rowLabel + "  -  " + colLabel + "</h5>");
			$('#overlaps-modal .modal-body').html("");

			results.forEach(function(result) {
				var content = "";
				content += "<table style='width:100%'>";
				
				var rowNbr = 1;
				result.rows.forEach( function(row) {
					content += 
						"<tr>" 
						+ "<td>" + rowNbr++ + ".</td>"
						+ "<td>" + "<a href='javascript:void(0)' onclick=\"loadUCSCTracks(" + "'" + row.chr + "'," + row.start + ',' + row.end + ")\">" +row.chr + ' ' + addCommas(row.start) + '-' +  addCommas(row.end) + "</a></td>"
						+ "</tr>";
				
				})
				content += "</table>";
				$('#overlaps-modal .modal-body').append(content);
			});



			$('#overlaps-modal').modal('show');
			
		  

	    },
	    error: function(error) {
	    	console.log(error);
	    }
	});	

}

function loadHeatmap() {

	var defUrl  = giggleUrl + "?data";
	var dataUrl = giggleUrl + "?region=" + $('#overlaps').val();


	// Get matrix definition
	sourceFileMap = {};	
	$.ajax({
	    url: defUrl,
	    type: "GET",
	    crossDomain: true,
	    dataType: "text",
	    success: function(data) {

	    	def = JSON.parse(data);
			def.sourceFiles.forEach( function( sourceFile ) {
				var cellCoord = {};
				cellCoord.row = sourceFile.position[0];
				cellCoord.col = sourceFile.position[1];
				sourceFileMap[sourceFile.name] = cellCoord;
				coordMap[cellCoord.row + "-" + cellCoord.col] = {rowLabel: def.dimensions[0].elements[cellCoord.row], colLabel: def.dimensions[1].elements[cellCoord.col]};
			});
			def.cells = [];

			var recordMap = {};

			// get matrix data (tab delimited) and fill in heatmap
			$.ajax({
			    url: dataUrl,
			    type: "GET",
			    crossDomain: true,
			    dataType: "text",
			    success: function(data) {
					data.split("\n").forEach(function(row) {
						fields = row.split("\t");
						if (fields.length == 0 || fields[0] == "") {

						} else {
							var rec = {};
							rec.name = fields[0].split("split/")[1];
							rec.size = fields[1];
							rec.overlaps = fields[2];
							rec.row = sourceFileMap[rec.name].row;
							rec.col = sourceFileMap[rec.name].col;
							def.cells.push(rec);

						}
					});

					maxValue = d3.max(def.cells, function(d,i) {return d.overlaps});
					if (maxValue <= 2) {
						maxValue = 3;
					}
					heatmap.colors(colorbrewer.YlGnBu[Math.min(maxValue, 9)])

					
				  	var selection = d3.select("#chart").datum(def);
		  			heatmap(selection);	

			    },
			    error: function(error) {

			    }
			});
		},
	    error: function(error) {
	    	console.error;
	    }
	});



}

function addCommas(nStr)
{
	nStr += '';
	x = nStr.split('.');
	x1 = x[0];
	x2 = x.length > 1 ? '.' + x[1] : '';
	var rgx = /(\d+)(\d{3})/;
	while (rgx.test(x1)) {
		x1 = x1.replace(rgx, '$1' + ',' + '$2');
	}
	return x1 + x2;
}