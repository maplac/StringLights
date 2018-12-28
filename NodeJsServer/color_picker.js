bg={};sR={};sG={};sB={};

function setBG(){
	c=[sR.value.toString(),sG.value.toString(),sB.value.toString()];
	bg.style.backgroundColor='rgb('+c[0]+','+c[1]+','+c[2]+')';
	clicked();
}

function setEL(s, t){
	s.addEventListener("change", function(){
	t.value=s.value;
	setBG();
},false);

t.addEventListener("change", function(){
	if(t.value>255)t.value=255;
	if(t.value<0)t.value=0;
	s.value=t.value;
	setBG();
	},false);
}

function setRanges(){
	var inputs=document.getElementsByTagName('input');
	for(var i=0;i<inputs.length;i++){
	inputs[i].min=0;
	inputs[i].max=255;
	inputs[i].value=0;
	}
}

function clickedLoad(but){
	var id=but.id.split("_")[1];
	c[0]=C[id][0];
	c[1]=C[id][1];
	c[2]=C[id][2];
	sR.value=c[0];
	sG.value=c[1];
	sB.value=c[2];
	tR.value=c[0];
	tG.value=c[1];
	tB.value=c[2];
	setBG();
}

function sendPost(url,msg){
var http=new XMLHttpRequest();
http.open("POST",url,true);
http.setRequestHeader("Content-type","application/x-www-form-urlencoded");
http.send(msg);
}
function clicked(but){
	sendPost("index.html","id=100"+"&r="+c[0]+"&g="+c[1]+"&b="+c[2]);
}
function clickedSave(but){
var id=but.id.split("_")[1];
C[id][0]=c[0];
C[id][1]=c[1];
C[id][2]=c[2];
document.getElementById('sp'+id).style.backgroundColor='rgb('+c[0]+','+c[1]+','+c[2]+')';
sendPost("staticColor.lua","id="+id+"&r="+c[0]+"&g="+c[1]+"&b="+c[2]);
}

html1='<div id="bg">\
<span class="red"><input id="sR"type="range"/></span>\
<input id="tR"type="number"/></br>\
<span class="gre"><input id="sG"type="range"/></span>\
<input id="tG"type="number"/></br>\
<span class="blu"><input id="sB"type="range"/></span>\
<input id="tB"type="number"/></br></div><div>';
window.onload=function(){
	t=html1;
	for(var i=0;i<C.length;i++){
		t=t+'<span id="sp'+i+'"class="spa">\
<button class="but_save"id="but_'+i+'"onclick="clickedSave(this)"><img src="icon_save.png"></button>\
<button class="but_load"id="but_'+i+'"onclick="clickedLoad(this)"><img src="icon_load.png"></button>\
		</span></br>';
	}
	t=t+'</div>';
	document.getElementById('color_menu').innerHTML+=t;
	for(var i=0;i<C.length;i++){
		document.getElementById('sp'+i).style.backgroundColor='rgb('+C[i][0]+','+C[i][1]+','+C[i][2]+')';
	}
	setRanges();
	bg=document.getElementById('bg');
	sR=document.getElementById('sR');
	sG=document.getElementById('sG');
	sB=document.getElementById('sB');
	tR=document.getElementById('tR');
	tG=document.getElementById('tG');
	tB=document.getElementById('tB');
	setEL(sR,tR);
	setEL(sG,tG);
	setEL(sB,tB);
	sR.value=c[0];
	sG.value=c[1];
	sB.value=c[2];
	tR.value=c[0];
	tG.value=c[1];
	tB.value=c[2];
	bg.style.backgroundColor='rgb('+c[0]+','+c[1]+','+c[2]+')';
	
}
