# SportVFR

!CATION! - Continued work in progress

An open-source DIY flight instrument

![SportVFR](images/Case%20Prototype.jpeg)

![SportVFR](images/sportvfr%20on%20panel.png)

Early Prototype (will post updates once v2 is ready, early April 2025)

![alt text](https://4.bp.blogspot.com/-GkP8dgFIXWg/WY2YKC37I7I/AAAAAAAAMsY/HeARyIRffEwuhb2vx854bXn1ijqGvWt4QCKgBGAs/s1600/20160601_071528.jpg "Sport VFR DIY project")

# V0.3 Update (1/21/2026)
![SportVFR Round](images/SportVFR_Round.jpeg)
![SportVFR Round Back](images/SportVFR_Round_Back.jpeg)

The most obvious and immediate change is that I switched to the higher speed and higher GPIO pin count on the STM32F103 "Blue Pill".

From feedback, I built a 3-1/8" Round version of the SportVFR, it's built from the identical schematic as the rectangular version. 

I'm preparing to build a flying version, which is an engine monitor only for now.  I will add the airspeed, compass, and altimeter soon, but focusing on the engine sensors and monitoring allows me to finalize them.

I struggled with the configuration so much that I just removed it for now.  It will work for the exact engine sensors I have, if you need to tweak the coefficients for your sensors or use different kinds of sensors, you'll have to change the code for now.  I am thinking about techniques for accommodating various sensors in the hardware and software, but for now, I'm keeping it simple.

Sensors... A constant struggle.  I have settled on a few basic things around sensors.

Fuel Quantity Sensors - Low impedance resistive.  They use a precision op-amp to convert low-current/voltage signals into the analog 0-3.3v signal for the ADC.  I have 4 channels of op-amps.

Oil Temperature - I'm using the Westech oil temp sensor.  It reads 10k-Ohms at 0C, 1.6k Ohms at 39C, 212 Ohms at 100C.  This is easily a low current (0.5ma @ 120C/250F) with a simple voltage divider using a high-precision pull-up resistor.  The code interpolates the logarithmic temperature response curve to give highly accurate temperatures.

Oil Pressure / Fuel Pressure - I use 3-wire pressure sensors; they are much easier to handle and just return voltages from 0.3 - 2.7v using a 3.3v source.  Super simple, also curve-fit-interpolated by the code from measured data.

Outside Air Temp - I used a Dallas one-wire temperature probe.  It's a digital response and highly accurate from -20C - 100C.  I 3d printed a fuse-skin mounting bracket.

![Oat Bracket](images/oat_bracket.jpeg)

The Tachometer and Fuel Flow are both pulse count sensors.  This is best done using the built-in hardware clock timers, the code is well documented.  This is not yet tested, and I'm sure there will be tweaks as I get into real-world testing. 

# V0.2 Update (3/31/2025)

![SportVFR0_2](images/SportVFR%20v0.2.png)
I've been doing a lot of work on this, and I've made several big improvements.

New Wiring Diagram
![SportVFR_0_2_Wiring](images/SportVFR%20v0.2%20Wiring.png)


1. There's now a DSub-15 pinout with more grounds. unfortunately, couldn't add all the 5V-Out for sensor power needed; there's just one.
1. Using op-amps to measure resistance sensors (OT, FQ1, FQ2), better all around
1. Integrated the magnetometer and absolute pressure sensors into the board design.  This lowers the project cost and makes the BOM smaller and the project simpler to build.
1. I've tested every sensor/instrument and gotten them all working
1. Code changes
 - Lots of simple fixes
 - Sensor-read-skip.  There's 7+ sensors to ready now, It's best to kkin sensor reads each cycle so the slow sensor reads update less frequently than the page display, big performance boost.
 - Created a fuel quantity page

# V0.3 Updates (tbd)

1. DSub25 with more grounds and 5v-Out for sensors
2. Aux1-5 with Aux1 pre-conditioned for low impedence sensors (another fuel quantity, etc)
3. Fuel Flow sensor added

# The Project

## Engine/Aircraft System Data Acquisition
There are only 8 analog input channels. That is limiting, but can support all required inputs;
  
|                          | Interface         | Sensor     |
|--------------------------|-------------------|------------|
| Airspeed                 | Serial - I2C      | ABPMRRV060MG2A3 |
| Altitude                 | Serial - I2C      | GZP6816D |
| Compass                  | Serial - I2C      | TLV493DA1B6HTSA2 |
| Engine RPM               | Counter 0-5v | Direct Messure  |
| Oil Temperature          | Analog Resistive  | Low-Impedence Amp  |
| Oil Pressure             | 0-5v  | 5V Pressure Gauge  |
| Fuel Pressure            | 0-5v  | 5V Pressure Gauge  |
| Fuel Quantity, each tank | Analog Resistive  | Low-Impedence Amp |
| Fuel Flow 		   | Counter 0-5v  | BM8563 |
| Tach Hour Meter	   | Uses Tach and time|            |

## BOM

### MCU - "Pro Micro atmega32u4"

### Compass and Altitude
  Built into the board

### Air Pressure sensors
[ABPMRRV060MG2A3](https://www.mouser.com/ProductDetail/Honeywell/ABPMRRV060MG2A3?qs=OTrKUuiFdkZ2B8eiLukmLQ%3D%3D)

*Alternatives (will require coding):*
- PX4 Airspeed Module (MS4525DO is the sensor)
- MPXV7002DP - This is a differential analog and may require amplification

### Engine Sensors

- Tach: This is a 2-pulse per rotation 12v signal coming from my Magnetos, it's voltage split to 0-5v.  You can change the number of pulses in the code.
- Oil Temp, Oil Pressure, Fuel Pressure, Fuel Quantity : These are all resistivity sensors with a typical operating range of 0-5kohm.  You can use any sensors that work on your engine.  You will need to calibrate them in the code.
  
### Display
HD44780 -I2C 1602 Character LCD Display

### Input
Rotary Encoder (CYT1100), usually comes with a small knob

### Alert LED
10mm Red LED

## Typical cost break-down (Spring 2025)

| Sport VFR             |             |         |
| --------------------- | ----------- | ------- |
|                       | Source      | Amount  |
| PCB Board             | email me    | $4.50  |
| Case                  | thingiverse | $10.00  |
| ProMicro              | Jungle      | $7.00   |
| Airspeed              | Mouser      | $35.00  |
| Rotary Encoder        | Jungle      | $0.56   |
| Alert LED / Resistors | Jungle      | $0.10   |
|                       |             |         |
|                       | Sub-Total   | $57.16  |
|                       |             |         |
| Sample Sensor BOM     |             |         |
|                       |             |         |
| Oil Pressure          | Jungle      | $14.68  |
| Oil Temperature       | Jungle      | $18.55  |
| Fuel Pressure         | Jungle      | $23.99  |
| Fuel Quantity x2      | Jungle      | $59.98  |
|                       |             |         |
|                       | Sub-Total   | $117.20 |
|                       |             |         |
| Total. System Cost    |             | $174.36 |

## The build

![SportVFR Board](images/sportvfr%20board.jpeg)



The [SportVFR v0.2 Board](scheamtic/SportVFR.0.0.2.zip) contains the Gerber files. These can be fabricated by multiple vendors.  My run of 5 was $20 shipped.  Most components are through-hole, but some are "large" format surface mounts that can be soldered by hand with medium soldering skills.  Installing the DB-9 right-angle connector is preferred so you can remove the display quickly later to work on your panel, but you may directly solder the input wires to the through-holes.

![SportVFR Board](images/sportvfr%20case.jpeg)

For the case, print it yourself or send it out.  It does require 4 6x8mm "threadserts" and M3 screws to mount to the panel.

## Testing and Calibration

!CATION! - Work in progress (see updates coming soon)

Once assembled and connected to sensors you will need to calibrate the system to your sensors.  This is done by compiling to the calibration mode and recording various values at known sensor points.  Please note the calibrations are unitless, you can use whatever units you want, so long as you remember them. Typical US units would be knots, gallons, psi and Fahrenheit... But you could calibrate in kph, liters, mPa and C or any combination so long as the numbers displayed on the screen make sense to you.

Calibration points/ commands are:

You enter compass calibration mode by holding down the encoder button while powering up the unit.  From there you'll be prompted to move the plane in a circle... just follow the prompts.

NOTE: When the in-code varaibles is set to "InCalibrationMode = true" The display is outputing raw values.  The layout is similar to but not identical to the normal display mode in order to show all sensors outputs.  It as as follows:
| Airspeed              | Heading  	| Altitude    |	 Up/down   |
| --------------------- | ----------- 	| ----------- |----------  |
| Tachometer 		| Oil Temp      | Oil Press   | Fuel Press | 


How to calibrate various sensors:  This calibration requires you to record values from the unconfigured device and enter them into the code.  You can always re-run these test by setting "InCongig" true and the values will get displayed.

- Airspeed: Read the zero pressure at rest.  Using a hose connected to the top tube of the airspeed sensor, have someone drive you a known speed and record that speed it to that speed.  Enter those values for AirspeedZero and Airspeed60
- Altimeter: Lookup the altitude of your current location.  Set the barometeric pressure to the closest known source and set the altitude in configuraton mode.
- Tachometer: Look up PulsesPerRPM in the code and set that to tachometer source. 
- Oil Temp: Set out a bowl of ice water and get a pot of boiling water, you'll use these test points to calibrate the temperature sensor.  Take the numbers displayed and put them into the code for OilTempMin and OilTempMax
- Oil Pressure & Fuel Pressure: Set the zero pressure at rest.  If your sensors have NPT (nation pipe thread) fittings, they will likely fit directly into a standard compressor hose.  Fill your tank to a known pressure within the range of the sensor and set it to that.  Take the numbers displayed and put them into the code as Oil/Fuel Pressure Min & Max.

# Future thoughts...

## Display or Not Display? That is the question. 

One could easily built an entire dedicated flight information display system (EFIS/ECAM) around this platform.

Or... You could say that any phone or tablet is a perfectly robust flight display system and what if you just focussed on all the data a phone/tablet can't get easily.  Airspeed, Compass, Engine, System sensors, etc.  The project needs to be integrated with a bluetooth transciever (HC-05).  Updates can easily be made to Avare for Android.  At some point compatability with other IMUs would be good so more wide adoption can be considered.

I honestly like both ideas but I lean toward the later as it's simpler and more direct.  Either way, I'd like to leave this project open-ended and not make any decisions that presuppose a display system.  

## IMU Considerations

* Sensors 
	The checked in code uses a HMC5883. The MPU9250 is newer, I would substitute.  This is a  raw output sensor and requires pretty complex 4-dof (quaternion) summing to output useful attitude data.  The altitude pressure sensor are adiquate and usually come with the attitude module.
* Cleanflight / Naze32 upgrade?
	  The Naze32 quad IMU is a nice complete inertia system with tons of great support from the quad copter community.  I have some great success setting it up in aircraft mode and interfacing via serial MultiWii protocol. It output all attitude and altitude data raw with very fast refresh.  It also supports GPS.

## AoA
    A sensitive airspeed sensor could be added for AoA and interface similarly to the airspeed 
