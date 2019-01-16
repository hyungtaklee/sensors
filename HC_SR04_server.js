var http = require('http');
var url = require('url');
var querystring = require('querystring');

var server = http.createServer(function(request, response) {
	var parsed_url = url.parse(request.url);
	var parsed_query = querystring.parse(parsed_url.query, '&', '=');
	
	console.log("received distance: " + parsed_query.distance);

	response.writeHead(200, {'Content-Type':'text/html'});
	response.end('successfully received');
});

server.listen(3000, function() {
	console.log('server is running...');
});
