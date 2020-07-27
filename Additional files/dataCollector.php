<?php

/*
  ESP32: send data to your Domain 

  Uses POST command to send DHT data to a designated website
  The circuit:
  * BME280
  * Post to Domain 
  
  Original version of this code by Stephan Borsay
  
  https://www.hackster.io/detox/send-esp8266-data-to-your-webpage-no-at-commands-7ebfec
  
*/

date_default_timezone_set("America/Indianapolis"); 
$TimeStamp = date("Y-m-d   h:i:sa");

   if( $_REQUEST["last"] ||  $_REQUEST["glat"] ||  $_REQUEST["glng"] ||  $_REQUEST["cel"] ||  $_REQUEST["humidity"] ||  
   $_REQUEST["cpressure"]  || $_REQUEST["rain5"]|| $_REQUEST["rain60"]|| $_REQUEST["rain24"]|| $_REQUEST["alt"]) 
   {
      echo " The Last Update was: ". $_REQUEST['last']."" ;
      echo " The Latitude is: ". $_REQUEST['gpdlat']. " Degrees ";
      echo " The Logitude is: ". $_REQUEST['glng']. " Minutes ";
	 echo " The Temperature is: ". $_REQUEST['cel']. " C. ";
	 echo " The Humidity is: ". $_REQUEST['Humidity']. " % ";
      echo " The Barometric Pressure is: ". $_REQUEST['cpressure']. " hpa  ";
      echo " The Rainfall per 5 Minute is: ". $_REQUEST['rain5']. " mm  ";
	 echo " The Rainfall per Hour is:  " . $_REQUEST['rain60']. " mm  ";
	 echo " The Rainfall per 24 Hours is:  " . $_REQUEST['rain24']. " mm  ";
	 echo " The Altitude is:  " . $_REQUEST['alt']. " Feet  ";

	 
   }
	  
	
$var1 = $_REQUEST['last']; 
$var2 = $_REQUEST['glat'];
$var3 = $_REQUEST['glng'];
$var4 = $_REQUEST['cel'];
$var5 = $_REQUEST['humidity'];
$var6 = $_REQUEST['cpressure'];
$var7 = $_REQUEST['rain5'];
$var8 = $_REQUEST['rain60'];
$var9 = $_REQUEST['rain24'];
$var10 = $_REQUEST['alt'];
$WriteMyRequest=


"<!DOCTYPE html>".
"<html><head><body><div class='centered'>".
"<p>".
"<p><h2>Treyburn Lakes<br>".
"Indianapolis, IN, US".
"</h2>".
"<p><h3>Last update:   " . $var1 . "</p>".
"<p> The Latitude is : " . $var2 . " Degrees </p>".
"<p>And the Longitutde is : " . $var3 . " Minutes </p>".
"<p>And the Temperature : "  . $var4 . " C.</p>".
"<p>And The Humidity is : "  . $var5 . " % </p>".
"<p>And The Barometric Pressure is : "  . $var6 . " hpa </p>".
"<p>And The Rainfall per 5 Minute is : "  . $var7 . " mm</p>".
"<p>And The Rainfall per Hour is : "  . $var8 . " mm</p>".
"<p>And The Rainfall per 24 Hour is : "  . $var9 . " mm</p>".
"<p>And The Elevation is : " . $var10 . " Feet</p>".
"<p></p>".
"<p>".
"<a href='https://forum.arduino.cc/index.php?topic=466867.0' style='color: #ffffff' >Project Discussion</a></h3></p>".
"</div></body></head></html>"; 

file_put_contents('dataDisplayer.html', $WriteMyRequest);

?>
