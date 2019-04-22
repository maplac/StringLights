
function clickedTurn(but){
	var splitted=but.id.split('_');
	var http=new XMLHttpRequest();
	var url="/index";
	var params="type=cmd&cmd="+splitted[1];
	sendPost(url, params);
}

function ipAddressStringToArray(str){
	if(!str){
		return [];
	}
	var arr = [];
	var splitted = str.split(".");
	if(splitted.length === 4){
		for(var i = 0; i < 4; i++){
			if (splitted[i] <= 255 && splitted[i] >=0){
				arr[i] = splitted[i];
			} else {
				return [];
			}
		}
	} else {
		return [];
	}
	return arr;
}

function ipAddressArrayToString(arr){
	var str = "";
	for(var i = 0; i < arr.length; i++){
		if(str.length > 0){
			str += ".";
		}
		str += arr[i];
	}
	return str;
}

function clickedSubmit(){
	var ledCount = document.getElementById("led_count").value;
	var wifiSsid = document.getElementById("wifi_ssid").value;
	var wifiPasswd = document.getElementById("wifi_passwd").value;
	var params="type=settings&led_count="+ledCount+"&wifi_ssid="+wifiSsid;
	if(!wifiSsid){
		document.getElementById("submit_label").innerHTML = "WiFi SSID can't be empty.";
		return;
	}
	if(wifiPasswd){
		params += "&wifi_passwd="+wifiPasswd;
	}
	//sendPost(url, params);
	var http=new XMLHttpRequest();
	var url="/index";
	http.open("POST",url,true);
	http.setRequestHeader("Content-type","application/x-www-form-urlencoded");
	http.onreadystatechange=function(){
		if(http.readyState==4){
			if(http.status==200){
				//console.log(http.responseText)
				var label = document.getElementById("submit_label");
				if(http.responseText === "OK"){
					label.innerHTML += "<br>Reply received. Settings saved.<br>Restart the device to apply changes.";
				}else{
					label.innerHTML += "<br>Reply received. Saving setting failed with error message:<br>"+http.responseText;
				}
			}else if (http.status==400){
				var label = document.getElementById("submit_label");
				label.innerHTML += "<br>Reply received. Saving setting failed with error message:<br>"+http.responseText;
			}
		}
	};
	http.send(params);
	var label = document.getElementById("submit_label");
	label.innerHTML = "Submitted. Waiting for reply...";
}

function clickedSubmitStatic(){
	var staticActive = document.getElementById("static_active").checked;
	var staticIp = ipAddressStringToArray(document.getElementById("static_ip").value);
	var staticSubnet = ipAddressStringToArray(document.getElementById("static_subnet").value);
	var staticGate = ipAddressStringToArray(document.getElementById("static_gate").value);
	var staticDns = ipAddressStringToArray(document.getElementById("static_dns").value);
	
	if(staticIp.length === 0){
		document.getElementById("submit_label_static").innerHTML = "IP address is not valid.";
		return;
	}
	if(staticSubnet.length === 0){
		document.getElementById("submit_label_static").innerHTML = "Subnet mask is not valid.";
		return;
	}
	if(staticGate.length === 0){
		document.getElementById("submit_label_static").innerHTML = "Gateway is not valid.";
		return;
	}
	if(staticDns.length === 0){
		document.getElementById("submit_label_static").innerHTML = "DNS is not valid.";
		return;
	}
	
	var params="type=settings_static&static_active="+staticActive;
	params += "&ip="+byteToString(staticIp[0])+byteToString(staticIp[1])+byteToString(staticIp[2])+byteToString(staticIp[3]);
	params += "&subnet="+byteToString(staticSubnet[0])+byteToString(staticSubnet[1])+byteToString(staticSubnet[2])+byteToString(staticSubnet[3]);
	params += "&gateway="+byteToString(staticGate[0])+byteToString(staticGate[1])+byteToString(staticGate[2])+byteToString(staticGate[3]);
	params += "&dns="+byteToString(staticDns[0])+byteToString(staticDns[1])+byteToString(staticDns[2])+byteToString(staticDns[3]);
		
	// send the post request
	var http=new XMLHttpRequest();
	var url="/index";
	http.open("POST",url,true);
	http.setRequestHeader("Content-type","application/x-www-form-urlencoded");
	http.onreadystatechange=function(){
		if(http.readyState==4){
			if(http.status==200){
				//console.log(http.responseText)
				var label = document.getElementById("submit_label_static");
				if(http.responseText === "OK"){
					label.innerHTML += "<br>Reply received. Settings saved.<br>Restart the device to apply changes.";
				}else{
					label.innerHTML += "<br>Reply received. Saving setting failed with error message:<br>"+http.responseText;
				}
			}else if (http.status==400){
				var label = document.getElementById("submit_label_static");
				label.innerHTML += "<br>Reply received. Saving setting failed with error message:<br>"+http.responseText;
			}
		}
	};
	http.send(params);
	var label = document.getElementById("submit_label_static");
	label.innerHTML = "Submitted. Waiting for reply...";
}

function colorChanged(){
	sendPost("index","type=color"+"&r="+cp.c[0]+"&g="+cp.c[1]+"&b="+cp.c[2]);
}

function ipArrayToString(ipArray){
	if(ipArray.length !== 4){
		return "unknown";
	}
	return ipArray[0] + "." + ipArray[1] + "." + ipArray[2] + "." + ipArray[3];
}

window.onload=function(){
	cp=JSON.parse(cpstr);
	cp.c = cp.C[0];
	
	var settingsTemp = JSON.parse(settingsstr);
	
	document.getElementById("last_ip_address").innerHTML = ipArrayToString(settingsTemp.last_ip_address);
	document.getElementById("led_count").value = settingsTemp.led_count;
	document.getElementById("wifi_ssid").value = settingsTemp.wifi_ssid;
	document.getElementById("static_active").checked = settingsTemp.static_active;
	document.getElementById("static_ip").value = ipAddressArrayToString(settingsTemp.static_ip);
	document.getElementById("static_subnet").value = ipAddressArrayToString(settingsTemp.subnet);
	document.getElementById("static_gate").value = ipAddressArrayToString(settingsTemp.gateway);
	document.getElementById("static_dns").value = ipAddressArrayToString(settingsTemp.dns);
	
	loadColorPicker();
}