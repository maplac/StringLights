function clickedTurn(but){
var splitted=but.id.split('_');
var http=new XMLHttpRequest();
var url="/index";
var params="type=cmd&cmd="+splitted[1];
sendPost(url, params);
/*http.open("POST",url,true);
http.setRequestHeader("Content-type","application/x-www-form-urlencoded");
http.onreadystatechange=function(){if(http.readyState==4&&http.status==200){}};
http.send(params);*/
}

function colorChanged(){
	sendPost("index","type=color"+"&r="+cp.c[0]+"&g="+cp.c[1]+"&b="+cp.c[2]);
}

window.onload=function(){
	cp=JSON.parse(cpstr);
	cp.c = cp.C[0];
	loadColorPicker();
}