
const String baseHtmlStart = "<html lang=\"en\">\
  <head>\
    <meta charset=\"utf-8\">\
    <meta name=\"viewport\" content=\"width=device-width\">\
    <title>BTC Ticker</title>\
    <style>\
      body {font-size:x-large; background-color: #f7931a; font-family: Calibri, Arial, Helvetica, Sans-Serif; color:#4d4d4d; text-align:center;padding:5px;}\
      .form-control {font-size:x-large; margin-bottom:5px;}\
      .btn {font-size:x-large; background-color: #4d4d4d; color:white;}\
    </style>\
  </head>\
  <body>\
  <h2>BTC Ticker</h2><br>";
const String baseHtmlEnd = "<\/body><\/html>";

const String postForms = "<form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/postform/\">\
                          YOUR WIFI NAME<br>\
                          <input class=\"form-control\" type=\"text\" name=\"ssid\" value=\"PODA_6417\"><br>\
                          YOUR WIFI PASSWORD<br>\
                          <input class=\"form-control\" type=\"password\" name=\"password\" value=\"724748037\"><br>\
                          <input class=\"btn\" type=\"submit\" value=\"Save connection\">\
                        </form>\
                        <br><a href=\"\/\">back</a>";

void handleRoot() {
  String message = WiFiMulti.run() == WL_CONNECTED ? "<div style=\"color:lime;background-color: #4d4d4d;padding:5px;\">CONNECTED<br>" + WiFi.SSID() + "</div>" : "<div style=\"color:red;background-color: #4d4d4d;padding:5px;\">DISCONNECTED</div>";
  message += "<br><a href=\"\/\">Refresh status</a>";
  message += "<br><br>";
  message += "<a href=\"\/setConnect/\">Set connect</a>";
  server.send(200, "text/html", baseHtmlStart + message + baseHtmlEnd);

}

void handleSetConnect()
{
  server.send(200, "text/html", baseHtmlStart + postForms + baseHtmlEnd);
}

void handleForm() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/html", "Method Not Allowed");
  } else {
 
    String message = "Saved! Refresh for 5s...<br><br>";
    message += "<meta http-equiv=\"refresh\" content=\"5; url=http://192.168.1.1\">";
    message += "<a href=\"\/\">back</a>";

    String wssid = server.arg(0);
    String wpswd = server.arg(1);
    Serial.println(wssid);
    Serial.println(wpswd);
    
    WiFiMulti.addAP(wssid.c_str(), wpswd.c_str());

    server.send(200, "text/html", baseHtmlStart + message + baseHtmlEnd );
  }
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  server.send(404, "text/html", baseHtmlStart + message + baseHtmlEnd );
}
