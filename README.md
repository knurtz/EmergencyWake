<img src="Images/Logo/EmergencyWake_Logo.png" width="400" alt="Emergency Wake" />

## Idea
A bedside alarm clock that uses a big red [emergency brake lever](https://www.google.com/search?tbm=isch&q=notbremse%20zug&tbs=imgo:1) as a snooze button.

To display the time, a vintage [VFD](https://en.wikipedia.org/wiki/Vacuum_fluorescent_display) is used.

An STM32 microcontroller and its RTC unit is used for tracking the time. It's in deep sleep most of the time.

Soundfiles from an SD card can be read by the microcontroller and played back through a small speaker.

Because of the VFD's relatively high power consumption, the clock can't run on battery power alone, so a USB-C connector is used for a 5 V power supply. 

In case of a power outage, a 3 V coin cell battery powers the microcontroller. In this scenario a small buzzer will be used instead of a wakeup melody, in order to use as little power as possible and to also notify the user, something's wrong. Also the display won't work.

Also, there will be support for a [DCF77](https://en.wikipedia.org/wiki/DCF77) radio clock module to automatically update the time.

A toggle switch allows programming two independent alarms. The times are set using a rotary encoder.


## Resources
- [Useful information](http://www.vwlowen.co.uk/arduino/vfd/vfd-clock.htm) about VFDs
- [A Guide to Fundamental VFD Operation](https://www.noritake-elec.com/technology/general-technical-information/vfd-operation)
- [Instructables page](https://www.instructables.com/id/A-Simple-Driver-for-VFD-Displays/) about controlling VFDs
- [Github project](https://github.com/AndruPol/ChibiOS-WavePlayer) about an STM32 wave player with ChibiOS
- [Github project](https://github.com/vanbwodonk/F4D_MP3) about an STM32 MP3 decoder
- [Another Github project](https://github.com/no111u3/stm32f4d_encoder/blob/master/main.c) about using the timer's encoder mode


## Components
- used emergency brake from eBay
- VFD from [pollin.de](https://www.pollin.de/p/vakuum-fluoreszenzdisplay-futaba-4-lt-46zb3-4-digit-121537) with additional parts:
  - [PT6312B](https://datasheet.lcsc.com/szlcsc/Princeton-Tech-PT6312B_C110225.pdf) VFD controller IC (requires negative voltages)
  - Alternative: UDN6118 VFD driver IC from eBay with no need for negative voltage
  - L9110 H-bridge for VFD filament
  - GL5516 light dependent resistor from eBay
- STM32F407VET6 microcontroller
- SD card holder from [Würth](https://www.we-online.de/katalog/datasheet/693071020811.pdf)
- DCF77 module from [pollin.de](https://www.pollin.de/p/dcf-empfangsmodul-dcf1-810054)
- CR2032 coin cell holder
- small buzzer
- toggle switch from [pollin.de](https://www.pollin.de/p/kippschalter-kn3-c-103a-1-polig-on-off-on-420023)
- rotary encoder from [reichelt.de](https://www.reichelt.de/drehimpulsegeber-15-impulse-30-rastungen-vertikal-stec11b03-p73913.html?r=1)
- MC34063 DCDC converter
- 5 V to 3.3 V step down regulator from [Würth](https://www.we-online.de/katalog/datasheet/173950378.pdf)
- [WM8960](https://datasheet.lcsc.com/szlcsc/Wolfson-Microelectronics-PLC-WM8960CGEFL-RV_C18752.pdf) audio codec with integrated class-D amplifier

## Usage
The clock will be in deep sleep most of the time. The display can be activated for about 10 seconds by tugging the emergency brake shortly.

While the time is being displayed, flipping the toggle switch into one of its two "on" positions allows setting the alarm times. In this mode, the alarms can each be (de-)activated by pulling the emergency lever once. A deactivated alarm will be represented by \_\_:\_\_ on the display.

Setting the alarm time is done by pressing the rotary encoder's push button to set the hours and then pressing it again to set the minutes. Alarm times are stored inside an EEPROM IC to avoid loss of settings after a power outage.

Once one alarm rings, pulling the emergency lever shortly will put it into snooze mode, pulling longer will deactivate the alarm.
