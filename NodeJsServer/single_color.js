function colorChanged(){
	repaintSelected();
	sendPost("single-color","type=color&"+"&r="+c[0]+"&g="+c[1]+"&b="+c[2]);
	//sendPost("color-picker","type=color&"+"&r="+c[0]+"&g="+c[1]+"&b="+c[2]);
}

function savedColorChanged(id){
	var a = document.getElementById("sel_"+id);
	a.style.backgroundColor = 'rgb('+C[id][0]+','+C[id][1]+','+C[id][2]+')';
	if (isColorLight(C[id])) {
		a.style.color='#1a1a1a';
	}else{
		a.style.color='#FF5733';
	}
	a.innerHTML = names[id];
	repaintSelected();
}

function repaintSelected(){
	var x = document.getElementById('sel_selected');
	var colorIndex = savedColorIndex(c);
	if(colorIndex >= 0){
		x.innerHTML = names[colorIndex];
	}else{
		x.innerHTML = "-";
	}
	x.style.backgroundColor='rgb('+c[0]+','+c[1]+','+c[2]+')';
	if (isColorLight(c)) {
		x.style.color = '#1a1a1a';
	}else{
		x.style.color = '#FF5733';
	}
}

function loadSingleColor(){

	var a, b, c, cs;
	cs = document.getElementById("custom-select");
	
	// visible selected item
	a = document.createElement("DIV");
	a.setAttribute("class", "select-selected");
	a.setAttribute("id", "sel_selected");
	a.style.backgroundColor='rgb('+Sc[0]+','+Sc[1]+','+Sc[2]+')';
	if (isColorLight(Sc)) {
		a.style.color='#1a1a1a';
	}else{
		a.style.color='#FF5733';
	}
	var colorIndex = savedColorIndex(Sc);
	if(colorIndex >= 0){
		a.innerHTML = names[colorIndex];
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
	for(var i=0;i<C.length;i++){
		c = document.createElement("DIV");
		c.setAttribute("id", "sel_"+i);
		c.style.backgroundColor = 'rgb('+C[i][0]+','+C[i][1]+','+C[i][2]+')';
		if (isColorLight(C[i])) {
			c.style.color='#1a1a1a';
		}else{
			c.style.color='#FF5733';
		}
		c.innerHTML = names[i];
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


