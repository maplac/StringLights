var history={};
history.mc = [];
history.selected = [];


function colorChanged(){
	sendPost("multi-color","type=color"+"&r="+cp.c[0]+"&g="+cp.c[1]+"&b="+cp.c[2]);
	setColor();
	//sendPost("color-picker","type=color&"+"&r="+cp.c[0]+"&g="+cp.c[1]+"&b="+cp.c[2]);
}

function setColor(){
	
	var a = document.getElementById("color_list");
	var b = a.getElementsByClassName("color_item_check");
	var c = a.getElementsByClassName("color_item_name");
	
	if((b.length !== c.length) && (c.length !== mc.length)){
		console.log("Error setColor(): length doesn't match");
		return;
	}
	
	var selected = [];
		
	// get all checked items
	for(var i = 0; i < b.length; i++){
		if(b[i].checked === true){
			selected.push(i);
		}
	}
	
	if(selected.length === 0){
		return;
	}
	
	saveHistory();
	
	for(var i = 0; i < selected.length; i++){
		var index = selected[i];
		//console.log("setColor(): checked index " + i);
		mc[index] = Array.from(cp.c);
		c[index].style.backgroundColor='rgb('+cp.c[0]+','+cp.c[1]+','+cp.c[2]+')';
		c[index].style.color=getTextColor(cp.c);
		var colorIndex = savedColorIndex(cp.c);
		if(colorIndex >= 0){
			c[index].innerHTML = cp.names[colorIndex];
		}else{
			c[index].innerHTML = ".";
			c[index].style.color='rgb('+cp.c[0]+','+cp.c[1]+','+cp.c[2]+')';
		}
	}

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
	
	if(selected.length === 0){
		return;
	}
	
	saveHistory();
	
	// insert new items
	for( var i = 0; i < selected.length; i++){
		selected[i] += i;
		mc.splice(selected[i]+1, 0, Array.from(cp.c));
	}
	
	// create new color_list
	createColorList();
	
	// recheck items
	b = a.getElementsByClassName("color_item_check");
	for( var i = 0; i < selected.length; i++){
		b[selected[i] + 1].checked = true;
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
	
	if(selected.length === b.length){
		console.log("Error clickedRemove(): cannot remove all.");
		return;
	}

	if(selected.length === 0){
		return;
	}
	
	saveHistory();
	
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
		if(selected[i] < b.length){
			b[selected[i]].checked = true;
		}
	}
}

function clickedUndo(){
	document.getElementById("but_undo").disabled = true;
	
	mc = [];
	for(var i = 0; i < history.mc.length; i++){
		mc.push(Array.from(history.mc[i]));
	}
	
	createColorList();
	
	var a = document.getElementById("color_list");
	var b = a.getElementsByClassName("color_item_check");
	
	for(var i = 0; i < history.selected.length; i++){
		b[history.selected[i]].checked = true;
	}
}

function saveHistory(){
	
	document.getElementById("but_undo").disabled = false;
	
	history.mc = [];
	for(var i = 0; i < mc.length; i++){
		history.mc.push(Array.from(mc[i]));
	}
	
	var a = document.getElementById("color_list");
	var b = a.getElementsByClassName("color_item_check");
	
	history.selected = [];
	for(var i = 0; i < b.length; i++){
		if(b[i].checked === true){
			history.selected.push(i);
		}
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
	var d = document.getElementById("select_n_offset");
	var N = c.value;
	var offset = d.value;
	
	for(var i = 0; i < b.length; i++){
		if(i < offset){
			b[i].checked = false;
		}else{
			if(((i-offset) % N) === 0){
				b[i].checked = true;
			}else{
				b[i].checked = false;
			}
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
  cp.c = Array.from(mc[0]);
  loadColorPicker();
  createColorList();
  
  saveHistory();
  document.getElementById("but_undo").disabled = true;
  
  var a = document.getElementById("select_n_number");
  a.addEventListener("change", function(){
    clickedSelectN();
  },false);
  a = document.getElementById("select_n_offset");
  a.addEventListener("change", function(){
    clickedSelectN();
  },false);
}

