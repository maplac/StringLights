function colorChanged(){
	//repaintSelected();
	sendPost("multi-color","type=color"+"&r="+cp.c[0]+"&g="+cp.c[1]+"&b="+cp.c[2]);
	//sendPost("color-picker","type=color&"+"&r="+cp.c[0]+"&g="+cp.c[1]+"&b="+cp.c[2]);
}

function clickedAdd(){
	var a = document.getElementById("color_list");
	var b = a.getElementsByClassName("color_item_check");
	var selected = [];
	
	// get all checked items
	for(var i = 0; i < b.length; i++){
		if(b[i].checked === true){
			selected.push(i);
		}
	}
	
	// insert new items
	for( var i = 0; i < selected.length; i++){
		selected[i] += i;
		mc.splice(selected[i]+1, 0, [cp.c[0], cp.c[1], cp.c[2]]);
	}
	
	// create new color_list
	createColorList();
	
	// recheck items
	b = a.getElementsByClassName("color_item_check");
	for( var i = 0; i < selected.length; i++){
		b[selected[i]].checked = true;
	}
}

function clickedRemove(){
	var a = document.getElementById("color_list");
	var b = a.getElementsByClassName("color_item_check");
	var selected = [];
	
	// get all checked items
	for(var i = 0; i < b.length; i++){
		if(b[i].checked === true){
			selected.push(i);
		}
	}
	
	// remove items
	for( var i = 0; i < selected.length; i++){
		selected[i] -= i;
		mc.splice(selected[i], 1);
	}
	
	// create new color_list
	createColorList();
	
	// recheck items
	b = a.getElementsByClassName("color_item_check");
	for( var i = 0; i < selected.length; i++){
		b[selected[i]].checked = true;
	}
}

function clickedSelectAll(){
	var a = document.getElementById("color_list");
	var b = a.getElementsByClassName("color_item_check");
	for(var i = 0; i < b.length; i++){
		b[i].checked = true;
	}
}

function clickedSelectNone(){
	var a = document.getElementById("color_list");
	var b = a.getElementsByClassName("color_item_check");
	for(var i = 0; i < b.length; i++){
		b[i].checked = false;
	}
}

function clickedSelectInvert(){
	var a = document.getElementById("color_list");
	var b = a.getElementsByClassName("color_item_check");
	for(var i = 0; i < b.length; i++){
		if(b[i].checked === true){
			b[i].checked = false;
		}else{
			b[i].checked = true;
		}
	}
}

function clickedSelectN(){
	var a = document.getElementById("color_list");
	var b = a.getElementsByClassName("color_item_check");
	var c = document.getElementById("select_n_number");
	var N = c.value;
	for(var i = 0; i < b.length; i++){
		if((i % N) === 0){
			b[i].checked = true;
		}else{
			b[i].checked = false;
		}
	}
}

function createColorList(){
	var a = document.getElementById("color_list");
	
	// remove all
	while (a.firstChild) {
		a.removeChild(a.firstChild);
	}
	
	// create new
	for(var i = 0; i < mc.length; i++){
		var b = document.createElement("DIV");
		b.setAttribute("class", "color_item");
		
		var c = document.createElement("SPAN");
		c.setAttribute("class", "color_item_num");
		c.innerHTML = (i+1).toString();
		b.appendChild(c);
		
		c = document.createElement("INPUT");
		c.setAttribute("class", "color_item_check");
		c.setAttribute("type", "checkbox");
		b.appendChild(c);
		
		c = document.createElement("SPAN");
		c.setAttribute("class", "color_item_name unselectable");
		c.style.backgroundColor='rgb('+mc[i][0]+','+mc[i][1]+','+mc[i][2]+')';
		c.style.color=getTextColor(mc[i]);
		var colorIndex = savedColorIndex(mc[i]);
		if(colorIndex >= 0){
			c.innerHTML = cp.names[colorIndex];
		}else{
			c.innerHTML = ".";
			c.style.color='rgb('+mc[i][0]+','+mc[i][1]+','+mc[i][2]+')';
			//c.classList.add("unselectable");
		}
		b.appendChild(c);
		a.appendChild(b);
	}
}

window.onload=function(){
  cp=JSON.parse(cpstr);
  var mcTemp = JSON.parse(mcstr);
  mc = mcTemp.mc;
  cp.c = mc[0];
  loadColorPicker();
  createColorList();
}

