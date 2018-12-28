function clicked(but){
var splitted=but.id.split('_');
var http=new XMLHttpRequest();
var url="/index.html";
var params="cmd="+splitted[1];
console.log("clicked")
http.open("POST",url,true);
http.setRequestHeader("Content-type","application/x-www-form-urlencoded");
http.onreadystatechange=function(){if(http.readyState==4&&http.status==200){}};
http.send(params);
}
