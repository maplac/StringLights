void handleTransparent(){
  digitalWrite(gpioLedProcessing, 1);
  Serial.println("Handling transparent");
  bool error = false;
  
  if(server.hasArg("type")){
    String type = server.arg("type");
    if (type == "single") {
      if (server.hasArg("index") && server.hasArg("color")) {
        int index = server.arg("index").toInt();
        String color = server.arg("color");
        if (index >= 0 && index < ledCount) {
          if (color.length() == 9) {
            unsigned char r = stringToNum(color.charAt(0), color.charAt(1), color.charAt(2));
            unsigned char g = stringToNum(color.charAt(3), color.charAt(4), color.charAt(5));
            unsigned char b = stringToNum(color.charAt(6), color.charAt(7), color.charAt(8));
            strip->SetPixelColor(index, RgbColor(r, g, b));
            strip->Show();
          } else {
            error = true;
            server.send(400,"text/html", "Color doesn't have lengh 9 characters.");
          }
        } else {
          error = true;
          server.send(400,"text/html", "Index is out of range.");
        }
      } else {
        error = true;
        server.send(400,"text/html", "Index or color is missing.");
      }
    } else if (type == "all") {
      if (server.hasArg("color")) {
        String color = server.arg("color");
        if (color.length() == 9) {
          unsigned char r = stringToNum(color.charAt(0), color.charAt(1), color.charAt(2));
          unsigned char g = stringToNum(color.charAt(3), color.charAt(4), color.charAt(5));
          unsigned char b = stringToNum(color.charAt(6), color.charAt(7), color.charAt(8));
          volatile int q = 0;
          for (int i = 0; i < ledCount; i++){
            strip->SetPixelColor(i, RgbColor(r, g, b));
          }
          strip->Show();
        } else {
          error = true;
          server.send(400,"text/html", "Color doesn't have lengh 9 characters.");
        }
      }
    } else if (type == "multi") {
      if(server.hasArg("i") && server.hasArg("r") && server.hasArg("g") && server.hasArg("b")){
        String ind = server.arg("i");
        String r = server.arg("r");
        String g = server.arg("g");
        String b = server.arg("b");
        if ( (ind.length() % 3) == 0 && (r.length() % 3) == 0 && (g.length() % 3) == 0 && (b.length() % 3) == 0){
          if( ind.length() == r.length() && r.length() == g.length() && g.length() == b.length()){
            if((r.length() / 3) <= ledCount || r.length() < 0){
              /*Serial.print("transparent-multi: ");Serial.print(ind);
              Serial.print(", ");Serial.print(r);
              Serial.print(", ");Serial.print(g);
              Serial.print(", ");Serial.println(b);*/
              unsigned char index, red, green, blue;
              for(int i = 0; i < r.length(); i += 3){
                //*/
                index = stringToNum(ind.charAt(i), ind.charAt(i + 1), ind.charAt(i + 2));
                red = stringToNum(r.charAt(i), r.charAt(i + 1), r.charAt(i + 2));
                green = stringToNum(g.charAt(i), g.charAt(i + 1), g.charAt(i + 2));
                blue = stringToNum(b.charAt(i), b.charAt(i + 1), b.charAt(i + 2));
                if (index >= 0 && index < ledCount) {
                  strip->SetPixelColor(index, RgbColor(red, green, blue));
                }//*/
              }
              strip->Show();
            } else {
              error = true;
              server.send(400,"text/html", "r/g/b/ length is out of range");
            }
          } else {
            error = true;
            server.send(400,"text/html", "i/r/g/b/ lengths do not match.");
          }
        } else {
          error = true;
          server.send(400,"text/html", "i/r/g/b/ length is not divisible by 3");
        }
      } else {
        error = true;
        server.send(400,"text/html", "i/r/g/b is missing.");
      }
    } else {
      error = true;
      server.send(400,"text/html", "Unknown type.");
    }
  } else {
    error = true;
    server.send(400,"text/html", "Type is missing.");
  }

  if (!error)
    server.send(200,"text/html", "OK");
    
  digitalWrite(gpioLedProcessing, 0);
}
