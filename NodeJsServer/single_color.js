function colorChanged(){
	//console.log("Cindex: "+Cindex);
	var s = document.getElementById("colorSelector");
	s.value=Cindex;
	sendPost("single-color","type=color&"+"&r="+c[0]+"&g="+c[1]+"&b="+c[2]);
	//sendPost("color-picker","type=color&"+"&r="+c[0]+"&g="+c[1]+"&b="+c[2]);
}

function loadSingleColor(){
	/*
	var t = '<option value="100"disabled>None</option>';
	for(var i=0;i<C.length;i++){
		t += '<option value="'+i+'">'+names[i]+'</option>';
	}
	var s = document.getElementById("colorSelector");
	s.innerHTML+=t;
	s.value=Cindex;
	s.addEventListener("change", function(){
		var s = document.getElementById("colorSelector");
		var value = s.options[s.selectedIndex].value;
		//console.log(value);
		if(value !== "100"){
			clickedLoad(document.getElementById("but_"+value));
		}
	},false);
	*/
	var x, i, j, selElmnt, a, b, c;
	var x = document.getElementsByClassName("custom-select");
	x = x[0];
	
	a = document.createElement("DIV");
	a.setAttribute("class", "select-selected");
	a.innerHTML = names[Cindex];
	x.appendChild(a);
	/* For each element, create a new DIV that will contain the option list: */
	b = document.createElement("DIV");
	b.setAttribute("class", "select-items select-hide");
	for(j=0;j<C.length;j++){
		/* Create a new DIV that will act as an option item: */
		c = document.createElement("DIV");
		c.setAttribute("id", "sel_"+j);
		c.innerHTML = names[j];
		b.appendChild(c);
	}
	b.addEventListener('click', function(e) {
		console.log(e.target.id);
		console.log(e);
		var x = document.getElementsByClassName("select-selected");
		x = x[0];
		x.innerHTML = e.target.innerText;
		var id=e.target.id.split("_")[1];
		clickedLoad(document.getElementById("but_"+id));
	});
	x.appendChild(b);
	a.addEventListener("click", function(e) {
		/* When the select box is clicked, close any other select boxes,
		and open/close the current select box: */
		e.stopPropagation();
		closeAllSelect(this);
		this.nextSibling.classList.toggle("select-hide");
		this.classList.toggle("select-arrow-active");
	});
}

window.onload=function(){
  loadColorPicker();
  loadSingleColor();
}

function closeAllSelect(elmnt) {
  /* A function that will close all select boxes in the document,
  except the current select box: */
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

/* If the user clicks anywhere outside the select box,
then close all select boxes: */
document.addEventListener("click", closeAllSelect); 


