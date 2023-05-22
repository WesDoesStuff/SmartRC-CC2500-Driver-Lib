# THIS IS STILL IN DEVELOPMENT

# SmartRC-CC2500-Driver-Lib_V0.1.0

![GitHub (Pre-)Release Date]()

Note: Find out about the laws in your country.
Use at your own risk.

Attention! based off of version 2.5.7 of the SmartRC-CC2500-Driver.

---------------------------------------------
Announcements / other
---------------------------------------------




---------------------------------------------
Install:
---------------------------------------------

Can be downloaded directly from the Arduino library manager. Just search for CC2500. (maybe?)

---------------------------------------------
Foreword:
---------------------------------------------
  "
    First of all, thanks to Elechouse that I can make the modified library accessible to everyone.

    Link: http://www.elechouse.com/elechouse/



    The library has been redesigned and some improvements have been made.

    Among other things, you can now also use the internal send / receive function.

    I would be happy to receive your suggestions for further examples from other libraries.

    All examples included are listed in the next field.
  "
  
---------------------------------------------
Containing examples:
---------------------------------------------


---------------------------------------------
Instructions / Description:
---------------------------------------------
This driver library can be used for many libraries that use a simple RF ASK module,
with the advantages of the CC2500 module.It offers many direct setting options as in 
SmartRF Studio and calculates settings such as MHz directly.

The most important functions at a glance:

CC2500.Init();		//Initialize the CC2500. Must be set first!

CC2500.setPA(PA);		//Set transmission power.

CC2500.setMHZ(MHZ);		//Set the basic frequency.

CC2500.SetTx();		//Set transmit on. 

CC2500.SetTx(MHZ);		//Sets transmit on and changes the frequency.

CC2500.SetRX();		//Set receive on.

CC2500.SetRx(MHZ);		//Sets receive on and changes the frequency.

CC2500.setRxBW(RXBW);		//Set Receive filter bandwidth		

CC2500.setGDO(GDO0, GDO2); 	//Set Gdo0 (tx) and Gdo2 (rx) for serial transmission function.


CC2500.setSpiPin(SCK, MISO, MOSI, CSN); //custom SPI pins. Set your own Spi Pins.Or to switch between multiple CC2500. Must be set before init and before changing the CC2500.

CC2500.setChannel(chnl); 	//Set Channel from 0 to 255. default = 0(basic frequency).

CC2500.setClb(fband, cal1, cal2); //Optionally enter Offset Callibration.

CC2500.setGDO0(GDO0);         //Sets Gdo0 for internal transfer function.

CC2500.addGDO0(GDO0, Modul);	//Adds Gdo0 for internal transfer function (multiple CC2500).

CC2500.addGDO(GDO0, GDO2, Modul);  //Adds Gdo0 (tx) and Gdo2 (rx) for serial transmission function (multiple CC2500).

CC2500.addSpiPin(SCK, MISO, MOSI, CSN, Modul);  //Adds Spi Pins for multiple CC2500.

CC2500.setModul(Modul);  //Switches between modules. from 0 to 5. So a maximum of 6.


The new features are not final and can be changed at any time.

All can also be used in a loop and are applied directly.

---------------------------------------------
Wiring:
---------------------------------------------

Basically the same as the CC1101. Will add compatibility for CC2500 boards with built in PA and LNA

Notes: A logic level converter is recommended for arduino. It also works well without. Use at your own risk.

<img src="https://github.com/LSatan/SmartRC-CC2500-Driver-Lib/blob/master/img/Nano_CC1101.png"/>

<img src="https://github.com/LSatan/SmartRC-CC2500-Driver-Lib/blob/master/img/MEGA_CC1101.png"/>

<img src="https://github.com/LSatan/SmartRC-CC2500-Driver-Lib/blob/master/img/Esp8266_CC1101.png"/>

<img src="https://github.com/LSatan/SmartRC-CC2500-Driver-Lib/blob/master/img/Esp32_CC1101.png"/>

<img src="https://github.com/LSatan/SmartRC-CC2500-Driver-Lib/blob/master/img/TXS0108E_CC1101.png"/>

<img src="https://github.com/LSatan/SmartRC-CC2500-Driver-Lib/blob/master/img/Wiring_CC1101.png"/>

---------------------------------------------
Donation
---------------------------------------------

If you like the library, I would be happy about a star.

you can support me with a donation.
https://paypal.me/WesDoesStuff

You can also support the creator of the cc1101 library that this was based off of.
https://www.paypal.me/LittleSatan666

Thank You!

---------------------------------------------
Changelog: SmartRC-CC2500-Driver-Lib_V0.1.0
---------------------------------------------
22.05.2022

Driver Library		:Modifying library to keep the same functionality and ease of use but to work with the CC2500

