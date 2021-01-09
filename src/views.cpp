#include <Arduino.h>

const char index_html[] PROGMEM =  R"(
<!DOCTYPE html>
<html>
   <head>
      <title>%NODE_NAME% - Brightness Control</title>
      <style>
         html{
         background-color: #a0b8bd;
         }
         body{
         overflow-y: auto;
         max-height: 100vh;
         }
         .border{
         border: 3px solid #6f8a8f;
         border-radius: 10px;
         }
         .column {
         float: left;
         width: 25%% ;
         margin-left: 6.25%% ;
         }
         .row:after {
         content: "";
         display: table;
         clear: both;
         }
         .center {
         text-align: center;
         }
         .fb{
         background-color: #8ca6ab;
         }
         .light_red{
         background-color: rgba(255, 0, 0, 0.6);
         border: 3px solid rgba(255, 0, 0, 0.8);
         border-radius: 10px;
         }
         .light_orange{
         background-color: rgba(250, 175, 0, 0.6);
         border: 3px solid rgba(250, 175, 0, 0.8);
         border-radius: 10px;
         }
         .light_blue{
         background-color: rgba(3, 177, 252, 0.6);
         border: 3px solid rgba(4, 153, 217, 0.8);
         border-radius: 10px;
         }
         .footer {
         position: fixed;
         left: 0;
         bottom: 0;
         width: 100%% ;
         background-color: #8ca6ab;
         color: white;
         text-align: center;
         }
      </style>
   </head>
   <body>
      <div class="row center">
         <div class="column">
            <form action="/devBehConfig" method="post" class="fb border">
               <h1>Device behavior</h1>
               <p>Change parameters as you wish</p>
               <label for="defBright">Default brightness [%%]:</label><br>
               <input type="text" id="defBright" name="defBright" value="%DEF_BRT%"><br><br>
               <label for="lightTime">Light time [s]:</label><br>
               <input type="text" id="lightTime" name="lightTime" value="%LIGHT_TIME%"><br><br>
               <label for="fadeInTime">Led fade in time [ms]:</label><br>
               <input type="text" id="fadeInTime" name="fadeInTime" value="%FADE_IN%"><br><br>
               <label for="fadeOutTime">Led fade out time [ms]:</label><br>
               <input type="text" id="fadeOutTime" name="fadeOutTime" value="%FADE_OUT%"><br><br>
               <input type="submit" value="Submit"><br><br>
            </form>
            <br>
            <form action="/reboot" method="post" class="light_orange">
               <br>
               <p><b>Reboot %NODE_NAME%</b></p>
               <input type="submit" value="Reboot"><br><br>
            </form>
            <br>
            <form method='POST' action='/update' class="light_blue" enctype='multipart/form-data'>
               <br>
               <p><b>OTA Update</b></p>
               <input type='file' name='update'>
               <input type='submit' value='Update'><br><br>
            </form>
         </div>
         <div class="column">
            <form action="/mqttConfig" method="post" class="fb border">
               <h1>MQTT Settings</h1>
               <p>Set mqtt parameters</p>
               <label for="nodeName">Node name</label><br>
               <input type="text" id="nodeName" name="nodeName" maxlength = "20" value="%NODE_NAME%"><br><br>
               <label for="mqttServer">MQTT Server</label><br>
               <input type="text" id="mqttServer" name="mqttServer" value="%MQTT_SERVER%"><br><br>
               <label for="mqttPort">MQTT Port</label><br>
               <input type="text" id="mqttPort" name="mqttPort" value="%MQTT_PORT%"><br><br>
               <label for="mqttUser">User</label><br>
               <input type="text" id="mqttUser" name="mqttUser" value="%MQTT_USER%"><br><br>
               <label for="mqttPwd">Password</label><br>
               <input type="password" id="mqttPwd" name="mqttPwd"><br><br>
               <input type="submit" value="Submit"><br><br>
            </form>
            <br>
            <form action="/discoverHA" method="post" class="fb border">
               <br>
               <p>Discover %NODE_NAME% in HomeAssistant</p>
               <input type="submit" value="Discover"><br><br>
            </form>
         </div>
         <div class="column">
            <form action="/devConfig" method="post" class="fb border">
               <h1>Device connection config</h1>
               <p>Change device config if needed</p>
               <p>MAC Address: <b>%MAC%</b></p>
               <p>IP Address: <b>%IP%</b></p>
               <label for="apSSID">AP SSID</label><br>
               <input type="text" id="apSSID" name="apSSID" value="%AP_SSID%"><br><br>
               <label for="apPwd">AP Password</label><br>
               <input type="password" id="apPwd" name="apPwd"><br><br>
               <input type="submit" value="Submit"><br><br>
            </form>
            <br>
            <form action="/factoryReset" method="post" class="light_red">
               <br>
               <p><b>Factory reset</b> <br> (This action can't be undone)</p>
               <input type="submit" value="Reset"><br><br>
            </form>
            <br>
            <form action="/removeHA" method="post" class="light_red">
               <br>
               <p><b>Remove %NODE_NAME% from HomeAssistant</b> <br> (This action can't be undone)</p>
               <input type="submit" value="Remove"><br><br>
            </form>
         </div>
      </div>
      <div class="footer">
         <p>Pueblo&copy;2021  <a href="https://github.com/Puebllo/node_pir_led_pwm">Github</a></p>
      </div>
   </body>
</html>
)";