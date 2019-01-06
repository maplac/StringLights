bg={};sR={};sG={};sB={};

function setBG(){
	c=[sR.value.toString(),sG.value.toString(),sB.value.toString()];
	bg.style.backgroundColor='rgb('+c[0]+','+c[1]+','+c[2]+')';
	colorChanged();
}

function setEL(s, t){
	s.addEventListener("change", function(){
	t.value=s.value;
	Cindex = 100;
	setBG();
	},false);
	t.addEventListener("change", function(){
	if(t.value>255)t.value=255;
	if(t.value<0)t.value=0;
	s.value=t.value;
	Cindex = 100;
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
	Cindex = id;
	setBG();
}

function sendPost(url,msg){
var http=new XMLHttpRequest();
http.open("POST",url,true);
http.setRequestHeader("Content-type","application/x-www-form-urlencoded");
http.onreadystatechange=function(){if(http.readyState==4&&http.status==200){/*console.log(http.responseText)*/}};
http.send(msg);
}

function clickedSave(but){
	var id=but.id.split("_")[1];
	var name = document.getElementById('sp'+id).innerHTML;
	var res = confirm("Save color "+name+"?");
	if(res !== true){return;}
	C[id][0]=c[0];
	C[id][1]=c[1];
	C[id][2]=c[2];
	document.getElementById('sp'+id).style.backgroundColor='rgb('+c[0]+','+c[1]+','+c[2]+')';
	var hsp = Math.sqrt(0.299*(C[id][0]*C[id][0])+0.587*(C[id][1]*C[id][1])+0.114*(C[id][2]*C[id][2]));
	if (hsp > 127.5) {
		document.getElementById('sp'+id).style.color='#1a1a1a';
	}else{
		document.getElementById('sp'+id).style.color='#FF5733';
	}
	sendPost("color-picker","type=save&id="+id+"&r="+c[0]+"&g="+c[1]+"&b="+c[2]);
}

function clickedRename(but){
	var id=but.id.split("_")[1];
	var strR = byteToString(C[id][0]);
	var strG = byteToString(C[id][1]);
	var strB = byteToString(C[id][2]);
	var res = prompt("Enter new name", strR+" "+strG+" "+strB);
	if(res === null || res === ""){	return;}
	if(res.length > 12){ res = res.substring(0,12);}
	document.getElementById('sp'+id).innerHTML = res;
	sendPost("color-picker","type=rename&id="+id+"&name="+res);
}
function byteToString(num){
	var str = num+"";
	while(str.length < 3) str = "0"+str;
	return str;
}

function loadColorPicker(){
	var t='<div id="bg">\
	<span class="red"><input id="sR"type="range"/></span>\
	<input id="tR"type="number"/></br>\
	<span class="gre"><input id="sG"type="range"/></span>\
	<input id="tG"type="number"/></br>\
	<span class="blu"><input id="sB"type="range"/></span>\
	<input id="tB"type="number"/></br></div>';
	document.getElementById('color_menu_top').innerHTML=t;
	t='<table>';
	for(var i=0;i<C.length;i++){
		t=t+'<tr>\
<td><button class="but_save"id="but_'+i+'"onclick="clickedSave(this)"><img src="icon_save.png" alt="save"></button></td>\
<td><button class="but_load"id="but_'+i+'"onclick="clickedRename(this)"><img src="icon_edit.png" alt="rename"></button></td>\
<td><button class="but_load"id="but_'+i+'"onclick="clickedLoad(this)"><img src="icon_load.png" alt="load"></button></td>\
<td><span id="sp'+i+'"class="color_text">'+names[i]+'</span></td>\
</tr>';
	}
	t=t+'</table>';
	document.getElementById('color_menu').innerHTML+=t;
	for(var i=0;i<C.length;i++){
		document.getElementById('sp'+i).style.backgroundColor='rgb('+C[i][0]+','+C[i][1]+','+C[i][2]+')';
		var hsp = Math.sqrt(0.299*(C[i][0]*C[i][0])+0.587*(C[i][1]*C[i][1])+0.114*(C[i][2]*C[i][2]));
		if (hsp > 127.5) {
			document.getElementById('sp'+i).style.color='#1a1a1a';
		}else{
			document.getElementById('sp'+i).style.color='#FF5733';
		}
	}
	setRanges();
	bg=document.getElementById('bg');
	sR=document.getElementById('sR');
	sG=document.getElementById('sG');
	sB=document.getElementById('sB');
	tR=document.getElementById('tR');
	tG=document.getElementById('tG');
	tB=document.getElementById('tB');
	if(Cindex == 100){
		sR.value=c[0];
		sG.value=c[1];
		sB.value=c[2];
		tR.value=c[0];
		tG.value=c[1];
		tB.value=c[2];
	}else{
		c[0]=C[Cindex][0];
		c[1]=C[Cindex][1];
		c[2]=C[Cindex][2];
		sR.value=c[0];
		sG.value=c[1];
		sB.value=c[2];
		tR.value=c[0];
		tG.value=c[1];
		tB.value=c[2];
	}
	setEL(sR,tR);
	setEL(sG,tG);
	setEL(sB,tB);
	bg.style.backgroundColor='rgb('+c[0]+','+c[1]+','+c[2]+')';
}
