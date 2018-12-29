function clickedTurn(but){
var splitted=but.id.split('_');
var http=new XMLHttpRequest();
var url="/index";
var params="cmd="+splitted[1];
sendPost(url, params);
/*http.open("POST",url,true);
http.setRequestHeader("Content-type","application/x-www-form-urlencoded");
http.onreadystatechange=function(){if(http.readyState==4&&http.status==200){}};
http.send(params);*/
}

function colorChanged(){
	sendPost("color-picker","type=color&"+"&r="+c[0]+"&g="+c[1]+"&b="+c[2]);
}

window.onload=function(){
	loadColorPicker();
}