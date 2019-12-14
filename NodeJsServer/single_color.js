var sc = [];
var scIndex = 0;

function colorChanged(){
	setColor();
	sendSettings();
}

function setColor(){
	var a = document.getElementById("coloritemname_"+scIndex);
	sc[scIndex] = Array.from(cp.c);
	a.style.backgroundColor='rgb('+cp.c[0]+','+cp.c[1]+','+cp.c[2]+')';
	a.style.color=getTextColor(cp.c);
	var colorIndex = savedColorIndex(cp.c);
	if(colorIndex >= 0){
		a.innerHTML = cp.names[colorIndex];
	}else{
		a.innerHTML = colorToString(cp.c);
	}
}

function sendSettings(){
	sendPost("single-color","type=color&index="+scIndex+"&r="+sc[scIndex][0]+"&g="+sc[scIndex][1]+"&b="+sc[scIndex][2]);
}

function savedColorChanged(id){// called from color picker when color is saved or renamed
	loadSingleColor();
}

function clickedRadio(e) {
	var id=e.target.id.split("_")[1];
	scIndex = id;
	console.log(id);
	sendSettings();
}

function colorItemNameDoubleClicked(e){
	var id=e.target.id.split("_")[1];
	colorPickerSetColor(sc[id]);
	e.stopPropagation();
}
	
function loadSingleColor(){
	var a = document.getElementById("color_list");
	
	// remove all
	while (a.firstChild) {
		a.removeChild(a.firstChild);
	}
	
	// create new
	for(var i = 0; i < sc.length; i++){
		var b = document.createElement("DIV");
		b.setAttribute("class", "color_item");
		
		var c = document.createElement("SPAN");
		c.setAttribute("class", "color_item_num unselectable");
		c.innerHTML = (i+1).toString();
		b.appendChild(c);
		
		c = document.createElement("INPUT");
		c.setAttribute("class", "color_item_check");
		c.setAttribute("type", "radio");
		c.setAttribute("name", "color_radio");
		c.setAttribute("id", "colorradio_"+i);
		c.addEventListener("change", clickedRadio, false);
		b.appendChild(c);
		
		c = document.createElement("SPAN");
		c.setAttribute("class", "checkmark unselectable");
		c.innerHTML = ".";
		b.appendChild(c);
		
		c = document.createElement("SPAN");
		c.setAttribute("class", "color_item_name unselectable");
		c.setAttribute("id", "coloritemname_" + i);
		c.addEventListener("dblclick", colorItemNameDoubleClicked, false);
		c.style.backgroundColor='rgb('+sc[i][0]+','+sc[i][1]+','+sc[i][2]+')';
		c.style.color=getTextColor(sc[i]);
		var colorIndex = savedColorIndex(sc[i]);
		if(colorIndex >= 0){
			c.innerHTML = cp.names[colorIndex];
		}else{
			c.innerHTML = colorToString(sc[i]);
		}
		b.appendChild(c);
		
		// label makes the whole line clickable for (un)checking the checkbox
		var d = document.createElement("LABEL");
		d.appendChild(b);
		a.appendChild(d);
	}
	document.getElementById("colorradio_"+scIndex).checked = true;
}

window.onload=function(){
  cp=JSON.parse(cpstr);
  var scTemp = JSON.parse(scstr);
  sc = scTemp.sc;
  
  scIndex = scTemp.index;
  loadColorPicker();
  loadSingleColor();
}
