
void handleRoot(){
  digitalWrite(gpioLedProcessing, 1);
  Serial.println("Handling root.");
  server.sendHeader("Location", "/index.html",true);   //Redirect to our html web page
  server.send(302, "text/plane","");
  digitalWrite(gpioLedProcessing, 0);
}

//=============================================================================================
void handleWebRequests(){
  digitalWrite(gpioLedProcessing, 1);
  //Serial.println("Handling WebRequests.");
  if(loadFromSpiffs(server.uri())){
    digitalWrite(gpioLedProcessing, 0);
    return;
  }
  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  Serial.println(message);
  digitalWrite(gpioLedProcessing, 0);
}

//=============================================================================================
bool loadFromSpiffs(String path){
  String dataType = "text/plain";
  if(path.endsWith("/")) path += "index.htm";
  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  
  else if(path.endsWith(".html")) dataType = "text/html";
  else if(path.endsWith(".htm")) dataType = "text/html";
  else if(path.endsWith(".css")) dataType = "text/css";
  else if(path.endsWith(".js")) dataType = "application/javascript";
  else if(path.endsWith(".png")) dataType = "image/png";
  else if(path.endsWith(".gif")) dataType = "image/gif";
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".ico")) dataType = "image/x-icon";
  else if(path.endsWith(".xml")) dataType = "text/xml";
  else if(path.endsWith(".pdf")) dataType = "application/pdf";
  else if(path.endsWith(".zip")) dataType = "application/zip";

  Serial.print("Requested file: ");
  Serial.print(path);
  Serial.print(". ");
  
  File dataFile = SPIFFS.open(path.c_str(), "r");

  if (!dataFile) {
      Serial.println("Opening failed.");
  }else{
    Serial.println();
  }
  
  if (server.hasArg("download")) dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
  }

  dataFile.close();
  return true;
}
