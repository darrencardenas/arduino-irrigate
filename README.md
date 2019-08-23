# arduino-irrigate

 ## Introduction
 
 This project contains code, schematics, build plans, and 3D models to construct an automated garden watering system.  The main components of the system are an [Arduino Uno Rev3 microcontroller board](https://store.arduino.cc/usa/arduino-uno-rev3), a [soil moisture sensor](https://vegetronix.com/Products/VH400/), and a [water solenoid valve](https://www.adafruit.com/product/997).  The water solenoid valve is connected to a garden hose and a suitable watering device, such as a sprinkler or sprayer.  I used an old garden hose with holes drilled into it.  
 
<p align="center">
<img src="media/installed.jpg" alt="Installed System"/>
</p>
<p align="center">
<img src="media/watering_20fps.gif" alt="Watering Video"/>
</p>   
 
 ## Operation
 At a set time interval, the Arduino checks the moisture sensor for a minimum value. If the reading is below the minimum value, the water valve is opened for a period of time.  The moisture readings are sent via [email](https://www.smtp2go.com/setupguide/arduino/) and [text message](https://www.lifewire.com/sms-gateway-from-email-to-sms-text-message-2495456).  The readings are also saved to a [publicly viewable IoT website](https://community.thingspeak.com/tutorials/arduino/send-data-to-thingspeak-with-arduino/) to store data and generate graphs.

<p align="center">
<img src="media/graph.png" alt="Moisture Graph" align="middle" />
&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp
<img src="media/text.png" alt="Text Message" align="middle" />
</p>

## Features
- Allows customizable intervals for soil moisture checks and watering
- Leverages waterproof enclosures and cabling
- Sends email and text message (SMS) status updates
- Provides data monitoring via website
- Supports WiFi for data transmission
- Powered by a single +12VDC source

<p align="center">
<img src="media/overview.jpg" alt="Top Down View" align="middle" />
</p>

## Schematic Diagram
<p align="center">
<a href="https://github.com/darrencardenas/arduino-irrigate/blob/master/build_plans/schematic_irrigate.pdf">
<img src="media/schematic_irrigate.png" />
</a>
 </p>




