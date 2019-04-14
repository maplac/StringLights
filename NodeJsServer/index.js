var lastIpAddress = "";
var ledCount = 0;
var wifiSsid = "";

function clickedTurn(but){
	var splitted=but.id.split('_');
	var http=new XMLHttpRequest();
	var url="/index";
	var params="type=cmd&cmd="+splitted[1];
	sendPost(url, params);
}

function clickedSubmit(){
	var http=new XMLHttpRequest();
	var url="/index";
	ledCount = document.getElementById("led_count").value;
	wifiSsid = document.getElementById("wifi_ssid").value;
	wifiPasswd = document.getElementById("wifi_passwd").value;
	var params="type=settings&ledcount="+ledCount+"&wifi_ssid="+wifiSsid;
	if(!wifiSsid){
		document.getElementById("submit_label").innerHTML = "WiFi SSID can't be empty.";
		return;
	}
	if(wifiPasswd){
		params += "&wifi_passwd="+wifiPasswd;
	}
	//sendPost(url, params);
	var http=new XMLHttpRequest();
	http.open("POST",url,true);
	http.setRequestHeader("Content-type","application/x-www-form-urlencoded");
	http.onreadystatechange=function(){
		if(http.readyState==4&&http.status==200){
			//console.log(http.responseText)
			var label = document.getElementById("submit_label");
			if(http.responseText === "OK"){
				label.innerHTML += "<br>Reply received. Settings saved.<br>Restart the device to apply changes.";
			}else{
				label.innerHTML += "<br>Reply received. Saving setting failed with error message:<br>"+http.responseText;
			}
		}
	};
	http.send(params);
	var label = document.getElementById("submit_label");
	label.innerHTML = "Submitted. Waiting for reply...";
}

function colorChanged(){
	sendPost("index","type=color"+"&r="+cp.c[0]+"&g="+cp.c[1]+"&b="+cp.c[2]);
}

window.onload=function(){
	cp=JSON.parse(cpstr);
	cp.c = cp.C[0];
	
	var settingsTemp = JSON.parse(settingsstr);
	lastIpAddress = settingsTemp.last_ip_address;
	ledCount = settingsTemp.led_count;
	wifiSsid = settingsTemp.wifi_ssid;
	
	document.getElementById("last_ip_address").innerHTML = lastIpAddress;
	document.getElementById("led_count").value = ledCount;
	document.getElementById("wifi_ssid").value = wifiSsid;
	
	loadColorPicker();
}