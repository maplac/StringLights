
express = require('express');  //web server
app = express();
server = require('http').createServer(app);

server.listen(80); //start the webserver
app.use(express.static('./'));

//app.use(express.bodyParser());
app.use(express.urlencoded());
//app.use(express.json());      // if needed

app.post('/color-picker', function(req, res){
	console.log("Processing POST /color-picker");
	console.log(req.body);
	/*var cmd = req.param('cmd', null);
	if(cmd !== null){
		console.log(cmd);
	}*/
	//res.end('OK');
	res.send('OK');
});
app.post('/index', function(req, res){
	console.log("Processing POST /index");
	console.log(req.body);
	res.send('OK');
});
app.post('/single-color', function(req, res){
	console.log("Processing POST /single-color");
	console.log(req.body);
	res.send('OK');
});
app.post('/multi-color', function(req, res){
	console.log("Processing POST /multi-color");
	console.log(req.body);
	res.send('OK');
});

function exit() {
	// free all resources
	console.log(" Exit");
	process.exit();
}
process.on('SIGINT', exit);