# SportVFR
An open-source DIY flight instrument

![alt text](https://4.bp.blogspot.com/-GkP8dgFIXWg/WY2YKC37I7I/AAAAAAAAMsY/HeARyIRffEwuhb2vx854bXn1ijqGvWt4QCKgBGAs/s1600/20160601_071528.jpg "Sport VFR DIY project")

I've had a smattering of interest in this project.  I have continued to dabble with it on and off.  Here are some thoughts.

# Project updates / thoughts.

## Display or Not Display? That is the question. 

One could easily built an entire dedicated flight information display system (EFIS/ECAM) around this platform.

Or... You could say that any phone or tablet is a perfectly robust flight display system and what if you just focussed on all the data a phone/tablet can't get easily.  Airspeed, Compass, Engine, System sensors, etc.


I honestly like both ideas but I lean toward the later as it's simpler and more direct.  Either way, I'd like to leave this project open-ended and not make any decisions that presuppose a display system.  

## IMU Considerations

* Sensors 
	  The checked in code uses a HMC5883. The MPU9250 is newer, I would substitute.  This is a 	  raw output sensor, and requires pretty complex 4-dof (quaternion) summing to output useful attitude data.  The altitude pressure sensor are adiquate and usually come with the attitude module.
* Cleanflight / Naze32 upgrade?
	  The Naze32 quad IMU is a nice complete inertia system with tons of great support from the quad copter community.  I have some great success setting it up in aircraft mode and interfacing via serial MultiWii protocol. It output all attitude and altitude data raw with very fast refresh.  It also supports GPS.

## Airspeed and AoA - "MPXV7002DP module"
  Pressure transducers with Op-Amp levelling output to 0-Vcc on small pitot-static pressure variance
  
## Engine/Aircraft System Data Acquisition
There are only 8 analog input channels. That is limiting, but can support all required inputs;

  
|                          | Interface         | Sensor     |
|--------------------------|-------------------|------------|
| Airspeed                 | Analog Voltage    | MPXV7002DP |
| Altitude                 | Serial - I2C      | BMP180     |
| Compass                  | Serial - I2C      | MPU9250    |
| Oil Temperature          | Analog Resistive  |            |
| Oil Pressure             | Analog Resistive  |            |
| Engine RPM               | Digital - Counter |            |
| Manifold Pressure, if CS | Analog Resistive  |            |
| Fuel Pressure            | Analog Resistive  |            |
| Fuel Quantity, each tank | Analog Resistive  |            |

	That leaves any or / all of these non-mandated data points commonly measured on most aircraft;

|                          | Interface         | Sensor     |
|--------------------------|-------------------|------------|
| Electric System Voltage  | Analog            |            |
| Ammeter                  | I2C               | ACS712ELC  |
| Cylinder Head 1-6        | Analog Resistive  |            |
| Exhaust Gas   1-6        | Analog Resistive  |            |
| Fuel Flow                | Digital - Counter |            |

## The board
![alt text](https://2.bp.blogspot.com/-hmfU_t731m4/WY2YKBQSxEI/AAAAAAAAMsY/hXmvPc_4McYqGNxrQ8MHipueo1vy2iaqQCKgBGAs/s1600/20160601_071439.jpg "Sport VFR Board")

I made an interfacing daughter-board for the Atmega32U4 Micro Pro.  Schematics, Board and Gerber files are included in this repo.  Have at it.

## Display Integration

The project needs to be integrated with a bluetooth transciever (HC-05).  Updates can easily be made to Avare for Android.  At some point compatability with other IMUs would be good so more wide adoption can be considered.
