var bg={},sR={},sG={},sB={},tR={},tG={},tB={};


function setBG(){
	cp.c=[sR.value.toString(),sG.value.toString(),sB.value.toString()];
	bg.style.backgroundColor='rgb('+cp.c[0]+','+cp.c[1]+','+cp.c[2]+')';
	if (typeof colorChanged == "function"){
		colorChanged();
	}
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
	cp.c[0]=cp.C[id][0];
	cp.c[1]=cp.C[id][1];
	cp.c[2]=cp.C[id][2];
	sR.value=cp.c[0];
	sG.value=cp.c[1];
	sB.value=cp.c[2];
	tR.value=cp.c[0];
	tG.value=cp.c[1];
	tB.value=cp.c[2];
	setBG();
}

function sendPost(url,msg){
var http=new XMLHttpRequest();
http.open("POST",url,true);
http.setRequestHeader("Content-type","application/x-www-form-urlencoded");
http.onreadystatechange=function(){if(http.readyState==4&&http.status==200){/*console.log(http.responseText)*/}};
http.send(msg);
}

function getTextColor(color){
	var hsp = Math.sqrt(0.299*(color[0]*color[0])+0.587*(color[1]*color[1])+0.114*(color[2]*color[2]));
	if (hsp > 127.5) {
		return '#1a1a1a';
	}else{
		return '#FF5733';
	}
}

function clickedSave(but){
	var id=but.id.split("_")[1];
	var name = document.getElementById('sp'+id).innerHTML;
	var res = confirm("Save color "+name+"?");
	if(res !== true){return;}
	cp.C[id][0]=cp.c[0];
	cp.C[id][1]=cp.c[1];
	cp.C[id][2]=cp.c[2];
	document.getElementById('sp'+id).style.backgroundColor='rgb('+cp.c[0]+','+cp.c[1]+','+cp.c[2]+')';
	document.getElementById('sp'+id).style.color=getTextColor(cp.C[id]);
	sendPost("color-picker","type=save&id="+id+"&r="+cp.c[0]+"&g="+cp.c[1]+"&b="+cp.c[2]);
	if (typeof savedColorChanged == "function"){
		savedColorChanged(id);
	}
}

function clickedRename(but){
	var id=but.id.split("_")[1];
	var strR = byteToString(cp.C[id][0]);
	var strG = byteToString(cp.C[id][1]);
	var strB = byteToString(cp.C[id][2]);
	var res = prompt("Enter new name", strR+" "+strG+" "+strB);
	if(res === null || res === ""){	return;}
	if(res.length > 12){ res = res.substring(0,12);}
	document.getElementById('sp'+id).innerHTML = res;
	cp.names[id] = res;
	sendPost("color-picker","type=rename&id="+id+"&name="+res);
		if (typeof savedColorChanged == "function"){
		savedColorChanged(id);
	}
}
function byteToString(num){
	var str = num+"";
	while(str.length < 3) str = "0"+str;
	return str;
}

function savedColorIndex(color){
	for(var i=0;i<cp.C.length;i++){
		if(cp.C[i][0] == color[0] && cp.C[i][1] == color[1] && cp.C[i][2] == color[2]){
			return i;
		}
	}
	return -1;
}

function loadColorPicker(){
	document.getElementById('color_menu_top').innerHTML='<div id="bg">\
	<span class="red"><input id="sR"type="range"/></span>\
	<input id="tR"type="number"/></br>\
	<span class="gre"><input id="sG"type="range"/></span>\
	<input id="tG"type="number"/></br>\
	<span class="blu"><input id="sB"type="range"/></span>\
	<input id="tB"type="number"/></br></div>';
	var t='<table>';
	for(var i=0;i<cp.C.length;i++){
		t=t+'<tr>\
<td><button class="but_save"id="but_'+i+'"onclick="clickedSave(this)"><img src="icon_save.png" alt="save"></button></td>\
<td><button class="but_load"id="but_'+i+'"onclick="clickedRename(this)"><img src="icon_edit.png" alt="rename"></button></td>\
<td><button class="but_load"id="but_'+i+'"onclick="clickedLoad(this)"><img src="icon_load.png" alt="load"></button></td>\
<td><span id="sp'+i+'"class="color_text">'+cp.names[i]+'</span></td>\
</tr>';
	}
	t=t+'</table>';
	document.getElementById('color_menu').innerHTML+=t;
	for(var i=0;i<cp.C.length;i++){
		document.getElementById('sp'+i).style.backgroundColor='rgb('+cp.C[i][0]+','+cp.C[i][1]+','+cp.C[i][2]+')';
		document.getElementById('sp'+i).style.color=getTextColor(cp.C[i]);
	}
	setRanges();
	bg=document.getElementById('bg');
	sR=document.getElementById('sR');
	sG=document.getElementById('sG');
	sB=document.getElementById('sB');
	tR=document.getElementById('tR');
	tG=document.getElementById('tG');
	tB=document.getElementById('tB');
	sR.value=cp.c[0];
	sG.value=cp.c[1];
	sB.value=cp.c[2];
	tR.value=cp.c[0];
	tG.value=cp.c[1];
	tB.value=cp.c[2];
	setEL(sR,tR);
	setEL(sG,tG);
	setEL(sB,tB);
	bg.style.backgroundColor='rgb('+cp.c[0]+','+cp.c[1]+','+cp.c[2]+')';
}
