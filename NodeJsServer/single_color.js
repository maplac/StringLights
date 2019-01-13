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
	a.setAttribute("id", "sel_selected");
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
		//console.log(e.target.id);
		//console.log(e);
		var id=e.target.id.split("_")[1];
		//var x = document.getElementsByClassName("select-selected");
		//x = x[0];
		var x = document.getElementById('sel_selected');
		x.innerHTML = e.target.innerText;
		x.style.backgroundColor=e.target.style.backgroundColor;//'rgb('+C[id][0]+','+C[id][1]+','+C[id][2]+')';
		x.style.color = e.target.style.color;
		/*var hsp = Math.sqrt(0.299*(C[id][0]*C[id][0])+0.587*(C[id][1]*C[id][1])+0.114*(C[id][2]*C[id][2]));
		if (hsp > 127.5) {
			x.style.color='#1a1a1a';
		}else{
			x.style.color='#FF5733';
		}*/
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
	
	for(var i=0;i<C.length;i++){
		document.getElementById('sel_'+i).style.backgroundColor='rgb('+C[i][0]+','+C[i][1]+','+C[i][2]+')';
		if (isColorLight(C[i])) {
			document.getElementById('sel_'+i).style.color='#1a1a1a';
		}else{
			document.getElementById('sel_'+i).style.color='#FF5733';
		}
	}
	var d = document.getElementById('sel_selected');
	if(SCindex > 0){
		d.style.backgroundColor = document.getElementById('sel_'+SCindex).style.backgroundColor;
		d.style.color = document.getElementById('sel_'+SCindex).style.color;
		d.style.innerHTML = document.getElementById('sel_'+SCindex).innerHTML;
	}else{
		d.style.backgroundColor='rgb('+Sc[0]+','+Sc[1]+','+Sc[2]+')';
		if (isColorLight(C[i])) {
			d.style.color='#1a1a1a';
		}else{
			d.style.color='#FF5733';
		}
		d.style.innerHTML = "None";
	}
	
	
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


