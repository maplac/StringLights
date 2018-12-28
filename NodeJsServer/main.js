
express = require('express');  //web server
app = express();
server = require('http').createServer(app);

server.listen(80); //start the webserver
app.use(express.static('./'));

//app.use(express.bodyParser());
app.use(express.urlencoded());
//app.use(express.json());      // if needed

app.post('/index.html', function(req, res){
	console.log("Processing POST /index.html");
	var cmd = req.param('cmd', null);
	if(cmd !== null){
		console.log(cmd);
	}
});


function exit() {
	// free all resources
	console.log(" Exit");
	process.exit();
}
process.on('SIGINT', exit);