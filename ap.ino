String css ="<style> body {margin: 0 auto; font-family:Calibri, sans-serif} form {padding: 10px; margin: 5px;color: #f8f9fa;} a {text-decoration: none; color: #f8f9fa;} .bg-secondary {background-color: #6c757d !important;} .bg-light { background-color: #f8f9fa !important;} .bg-dark{background-color: #212529 !important;} .navbar {padding: 10px; color: #f8f9fa;} .navbar-text{text-align: right;} .btn {padding: 10px 10px 8px 10px;margin: 5px; display: inline-block; border: 1px solid rgb(105, 105, 105);color: black; background-color:#ffc107 ;} .container{padding: 5px;} .card{margin: 5px;border: 1px solid #212529;} .card-header,.card-body{padding: 10px; color: #f8f9fa; } .fw-bolder{font-weight: bolder;} .pt-3{padding-top: 8px;} .mb-3{margin-bottom: 8px;} .border{border:1px solid #212529;} .rounded,.btn,.card{border-radius: 3px;} .text-center{text-align: center;} .alert{border-radius: 3px;margin: 5px;} .alert-success{padding: 15px; background-color: #2cdb89; color:#0c3f27;} .alert-danger{padding: 15px; background-color: #dd7883; color:#6e1922;} .form-label,.form-control{display: block; padding: 5px;} .form-control{border-radius: 3px; border: 1px solid darkgrey;font-size: 18px; background-color: #6c757d; color: #f8f9fa;} </style>";
String baseHtmlStart  ="<!DOCTYPE html> <html lang=\"en\"> <meta charset=\"utf-8\"> <meta name=\"viewport\" content=\"width=device-width\"> <title>BTC Ticker</title>"
       + css + "</head><body class=\"bg-secondary\"> <nav class=\"navbar bg-dark\"> <a class=\"navbar-brand\" href=\"/\">BTC Ticker</a> <span class=\"navbar-text\"> - version: 1.0.0</span> </nav> <div class=\"container pt-3\">";
    
const String baseHtmlEnd = "<\/div><\/body><\/html>";
const String postForms ="<a class=\"btn mb-3\" href=\"/\">BACK</a> <form class=\"border rounded p-2 bg-dark\" method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/postform/\"> <div class=\"mb-3\"> <label for=\"ssid\" class=\"form-label\">Your WiFi name</label> <input type=\"ssid\" class=\"form-control\" id=\"ssid\" placeholder=\"My_WiFi\"> </div> <div class=\"mb-3\"> <label for=\"password\" class=\"form-label\">Your WiFi password</label> <input type=\"password\" class=\"form-control\" id=\"password\" placeholder=\"My_WiFi_Password\"> </div> <input class=\"btn btn-warning\" type=\"submit\" value=\"Save\"> </form>";

void handleRoot() {

  String message ="<meta http-equiv=\"refresh\" content=\"5; url=http://192.168.1.1\"> <a class=\"btn mb-3\" href=\"/setConnect/\">SETTINGS WiFi</a> <div class=\"card text-center \"> <div class=\"card-header bg-dark\">WiFi conection status</div>";
  
  if(WiFiMulti.run() == WL_CONNECTED){
     message += "<div class=\"card-body alert-success text-center\">Connected to <div class=\"fw-bolder\">" + WiFi.SSID() + "</div>Reload after 5 seconds... </div>";
   }else{
     message +="<div class=\"card-body alert-danger text-center\"> <div class=\"fw-bolder\">Not connected!</div> Please setting WiFi. </div>"; 
   }

   message += "</div>";
  
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
 
    String message ="<meta http-equiv=\"refresh\" content=\"5; url=http://192.168.1.1\"> <a class=\"btn mb-3\" href=\"/\">BACK</a> <div class=\"alert alert-success text-center \" role=\"alert\">WiFi settings saved.<br>Redirect to main page after 5 seconds or click back button.</div>";

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
