function colorChanged(){
	repaintSelected();
	sendPost("single-color","type=color"+"&r="+cp.c[0]+"&g="+cp.c[1]+"&b="+cp.c[2]);
	//sendPost("color-picker","type=color&"+"&r="+cp.c[0]+"&g="+cp.c[1]+"&b="+cp.c[2]);
}

function savedColorChanged(id){
	var a = document.getElementById("sel_"+id);
	a.style.backgroundColor = 'rgb('+cp.C[id][0]+','+cp.C[id][1]+','+cp.C[id][2]+')';
	a.style.color=getTextColor(cp.C[id]);
	a.innerHTML = cp.names[id];
	repaintSelected();
}

function repaintSelected(){
	var x = document.getElementById('sel_selected');
	var colorIndex = savedColorIndex(cp.c);
	if(colorIndex >= 0){
		x.innerHTML = cp.names[colorIndex];
	}else{
		x.innerHTML = "-";
	}
	x.style.backgroundColor='rgb('+cp.c[0]+','+cp.c[1]+','+cp.c[2]+')';
	x.style.color = getTextColor(cp.c);
}

function loadSingleColor(){

	var a, b, c, cs;
	cs = document.getElementById("custom-select");
	
	// visible selected item
	a = document.createElement("DIV");
	a.setAttribute("class", "select-selected");
	a.setAttribute("id", "sel_selected");
	a.style.backgroundColor='rgb('+cp.c[0]+','+cp.c[1]+','+cp.c[2]+')';
	a.style.color=getTextColor(cp.c);
	var colorIndex = savedColorIndex(cp.c);
	if(colorIndex >= 0){
		a.innerHTML = cp.names[colorIndex];
	}else{
		a.innerHTML = "-";
	}
	a.addEventListener("click", function(e) {
		// When the select box is clicked, close any other select boxes, and open/close the current select box:
		e.stopPropagation();
		closeAllSelect(this);
		this.nextSibling.classList.toggle("select-hide");
		this.classList.toggle("select-arrow-active");
	});
	cs.appendChild(a);
	
	// items in the option list
	b = document.createElement("DIV");
	b.setAttribute("class", "select-items select-hide");
	for(var i=0;i<cp.C.length;i++){
		c = document.createElement("DIV");
		c.setAttribute("id", "sel_"+i);
		c.setAttribute("class", "unselectable");
		c.style.backgroundColor = 'rgb('+cp.C[i][0]+','+cp.C[i][1]+','+cp.C[i][2]+')';
		c.style.color=getTextColor(cp.C[i]);
		var colorIndex = savedColorIndex(cp.C[i]);
		console.log(colorIndex);
		if(colorIndex >= 0){
			c.innerHTML = cp.names[colorIndex];
		}else{
			c.innerHTML = colorToString(cp.C[i]);
		}
		b.appendChild(c);
	}
	// add event listener for clicking on an item in the list
	b.addEventListener('click', function(e) {
		var id=e.target.id.split("_")[1];
		clickedLoad(document.getElementById("but_"+id));
	});
	cs.appendChild(b);

	var d = document.getElementById('sel_selected');// stejne jako "a"
}

window.onload=function(){
  cp=JSON.parse(cpstr);
  var scTemp = JSON.parse(scstr);
  sc = scTemp.sc;
  cp.c = Array.from(sc);
  loadColorPicker();
  loadSingleColor();
}

function closeAllSelect(elmnt) {
  var x, y, i, arrNo = [];
  x = document.getElementsByClassName("select-items");
  y = document.getElementsByClassName("select-selected");
  for (i = 0; i < y.length; i++) {
    if (elmnt == y[i]) {
      arrNo.push(i)
    } else {
      y[i].classList.remove("select-arrow-active");
    }
  }
  for (i = 0; i < x.length; i++) {
    if (arrNo.indexOf(i)) {
      x[i].classList.add("select-hide");
    }
  }
}

document.addEventListener("click", closeAllSelect); 


