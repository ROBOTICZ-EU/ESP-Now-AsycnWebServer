//index1.h --Weather web page
const char HTML1[] PROGMEM = R"====(
<!DOCTYPE HTML>
<html>

<head>
    <title>Weather Observations</title>
</head>

<body>
    <h2>Rain Gauge<br>Indianapolis, Indiana 46239</h2>
    <br> LastUpdate: %LASTUPDATE%
    <br> Latitude: %GPSLAT%
    <br> Longitude : %GPSLNG%
    <br> Elevation %GPSALT% Feet.
    <br> Temperature: %TEMP% F.
    <br> Humidity: %HUM% %%
    <br> Barometric Pressure: %PRESSURE% inHg
    <br> Difference:  %DIF% inHg --Past 15 Minutes
    <br> Rain Day : %RAINDAY% Day/mm
    <br> Rain Hour: %RAINHOUR% Hour/mm
    <br> Rain 5 Min : %RAINFALL% Five Min/mm
    <br>
    <h2>Weather Observations</h2>
    <h3>%DTSTAMP%</h3>
    <br>
    <a href=http://%LINK%/SdBrowse >File Browser</a>
    <br>
    <a href=http://%LINK%/Graphs>Graphed Weather Observations</a>
    <br>
    <a href=http://%LINK%/README.TXT >Readme</a>
    <br>
    <a href=http://%LINK%/SERVER.TXT >Server</a>
    <br>
    <a href=http://%LINK%/WATCHDOG.TXT >Watchdog</a>
    <br>
    <a href=http://%LINK%/WIFI.TXT >WiFi</a>
    <br>
    <a href=http://%LINK%/Contactus >Contact us</a>
    <br>
    <a href=https://forum.arduino.cc/index.php?topic=606947.0>Project Discussion</a>
    <br>
    <br> Client IP: %CLIENTIP%
</body>

</html>
)====";
