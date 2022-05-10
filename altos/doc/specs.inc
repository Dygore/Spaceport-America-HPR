[appendix]
== Altus Metrum Hardware Specifications

	Here's the full set of Altus Metrum products, both in
	production and retired.

	.Altus Metrum Flight Computer Electronics
	[options="header"]
	|===
	|Device | Barometer | Z-axis accel | GPS | 3D sensors | Storage | RF Output | Battery

	ifdef::telemetrum[]
	|TeleMetrum v1.0
	|MP3H6115 10km (33k')
	|MMA2202 50g
	|SkyTraq
	|-
	|1MB
	|10mW
	|3.7V

	|TeleMetrum v1.1
	|MP3H6115 10km (33k')
	|MMA2202 50g
	|SkyTraq
	|-
	|2MB
	|10mW
	|3.7V

	|TeleMetrum v1.2
	|MP3H6115 10km (33k')
	|ADXL78 70g
	|SkyTraq
	|-
	|2MB
	|10mW
	|3.7V

	|TeleMetrum v2.0
	|MS5607 30km (100k')
	|MMA6555 102g
	|uBlox Max-7Q
	|-
	|8MB
	|40mW
	|3.7V

	|TeleMetrum v3.0
	|MS5607 30km (100k')
	|ADXL375 200g
	|uBlox Max-8Q
	|-
	|8MB
	|40mW
	|3.7V

	endif::telemetrum[]
	ifdef::telemini[]
	|TeleMini v1.0
	|MP3H6115 10km (33k')
	|-
	|-
	|-
	|5kB
	|10mW
	|3.7V

	|TeleMini v3.0
	|MS5607 30km (100k')
	|-
	|-
	|-
	|512kB
	|40mW
	|3.7V

	endif::telemini[]
	ifdef::easymini[]
	|EasyMini v1.0
	|MS5607 30km (100k')
	|-
	|-
	|-
	|1MB
	|-
	|3.7-12V

	|EasyMini v2.0
	|MS5607 30km (100k')
	|-
	|-
	|-
	|1MB
	|-
	|3.7-12V

	endif::easymini[]
	ifdef::telemega[]
	|TeleMega v1.0
	|MS5607 30km (100k')
	|MMA6555 102g
	|uBlox Max-7Q
	|MPU6000 HMC5883
	|8MB
	|40mW
	|3.7V

	|TeleMega v2.0
	|MS5607 30km (100k')
	|MMA6555 102g
	|uBlox Max-7Q
	|MPU6000 HMC5883
	|8MB
	|40mW
	|3.7V

	|TeleMega v3.0
	|MS5607 30km (100k')
	|MMA6555 102g
	|uBlox Max-7Q
	|MPU9250
	|8MB
	|40mW
	|3.7V

	|TeleMega v4.0
	|MS5607 30km (100k')
	|ADXL375 200g
	|uBlox Max-8Q
	|BMX160
	|8MB
	|40mW
	|3.7V

	|TeleMega v5.0
	|MS5607 30km (100k')
	|ADXL375 200g
	|uBlox Max-8Q
	|MPU6000 MMC5983
	|8MB
	|40mW
	|3.7V

	endif::telemega[]
	ifdef::easymega[]
	|EasyMega v1.0
	|MS5607 30km (100k')
	|MMA6555 102g
	|-
	|MPU6000 HMC5883
	|8MB
	|-
	|3.7V

	|EasyMega v2.0
	|MS5607 30km (100k')
	|ADXL375 200g
	|-
	|MPU9250
	|8MB
	|-
	|3.7V
	endif::easymega[]

	ifdef::easytimer[]
	|EasyTimer v1.0
	|-
	|16g
	|-
	|BMX160
	|-
	|-
	|3.7-12V
	endif::easytimer[]
	
	ifdef::easymotor[]
	|EasyMotor v2.0
	|-
	|ADXL375 200g
	|-
	|-
	|-
	|-
	|6.5-15V
	endif::easymotor[]
	
	
	|===

	<<<<
	.Altus Metrum Flight Computer Mechanical Components
	[options="header",grid="all"]
	|===
	|Device|Connectors|Screw Terminals|Width|Length|Tube Size

	ifdef::telemetrum[]
	|TeleMetrum
	|Antenna Debug Companion USB Battery
	|Apogee pyro Main pyro Switch
	|1 inch (2.54cm)
	|2 ¾ inch (6.99cm)
	|29mm coupler
	endif::telemetrum[]

	ifdef::telemini[]
	|TeleMini v1.0
	|Antenna Debug Battery
	|Apogee pyro Main pyro
	|½ inch (1.27cm)
	|1½ inch (3.81cm)
	|18mm coupler

	|TeleMini v2.0
	|Antenna Debug USB Battery
	|Apogee pyro Main pyro Battery Switch
	|0.8 inch (2.03cm)
	|1½ inch (3.81cm)
	|24mm coupler
	endif::telemini[]

	ifdef::easymini[]
	|EasyMini
	|Debug USB Battery
	|Apogee pyro Main pyro Battery
	|0.8 inch (2.03cm)
	|1½ inch (3.81cm)
	|24mm coupler
	endif::easymini[]

	ifdef::telemega[]
	|TeleMega
	|Antenna Debug Companion USB Battery
	|Apogee pyro Main pyro Pyro A-D Switch Pyro battery
	|1¼ inch (3.18cm)
	|3¼ inch (8.26cm)
	|38mm coupler
	endif::telemega[]

	ifdef::easymega[]
	|EasyMega
	|Debug Companion USB Battery
	|Apogee pyro Main pyro Pyro A-D Switch Pyro battery
	|1¼ inch (3.18cm)
	|2¼ inch (5.62cm)
	|38mm coupler
	endif::easymega[]

	ifdef::easytimer[]
	|EasyTimer
	|Debug USB Battery
	|Pyro A Pyro B Battery
	|0.8 inch (2.03cm)
	|1½ inch (3.81cm)
	|24mm coupler
	endif::easytimer[]

	ifdef::easymotor[]
	|EasyMotor
	|Debug USB
	|+5V Pres GND Switch Battery
	|0.8 inch (2.03cm)
	|1½ inch (3.81cm)
	|24mm coupler
	endif::easymotor[]

	|===
