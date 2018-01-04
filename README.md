# SportVFR
An open-source DIY flight instrument

I've had a smattering of interest in this project.  I have continued to dabble with it on and off.  Here are some thoughts.

# Project updates / thoughts.

* Display or Not Display? That is the question. 

One could easily built an entire dedicated flight information display system (EFIS/ECAM) around this platform.

Or... You could say that any phone or tablet is a perfectly robust flight display system and what if you just focussed on all the data a phone/tablet can't get easily.  Airspeed, Compass, Engine, System sensors, etc.


I honestly like both ideas but I lean toward the later as it's simpler and more direct.  Either way, I'd like to leave this project open-ended and not make any decisions that presuppose a display system.  

* IMU Considerations

 ##Sensors - 
  The checked in code uses a HMC5883. The MPU9250 is newer, I would substitute.  This is a 
  raw output sensor, and requires pretty complex 4-dof (quaternion) summing to output useful attitude data.  The altitude pressure sensor are adiquate and usually come with the attitude module.

 ##Cleanflight / Naze32 upgrade?

  The Naze32 quad IMU is a nice complete inertia system with tons of great support from the quad copter community.  I have some great success setting it up in aircraft mode and interfacing via serial MultiWii protocol. It output all attitude and altitude data raw with very fast refresh.  It also supports GPS.

* Airspeed and AoA - "MPXV7002DP module"
  Pressure transducers with Op-Amp levelling output to 0-Vcc on small pitot-static pressure variance
  
* Engine/Aircraft System Data Acquisition
  There are only 8 analog input channels. That is limiting, but can support all required inputs;
  
	- Oil Temperature
	- Oil Pressure
	- Electrical System Voltage
	- Engine RPM (digital input)
	- Fuel Pressure
	- Fuel Quantity each tank
	- Manifold pressure (only required if not fixed-pitch propeller)

	That leaves any or / all of these non-mandated data points commonly measured on most aircraft;
	- Ammeter
	- Cylinder Head Temperature (x1-6)
	- Exhaust Gas Temperature (x1-6)
	- Fuel Flow (digital input)
