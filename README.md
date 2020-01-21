# EmergencyWake
Alarm clock with an emergency brake lever as a snooze button

## Idea
A bedside alarm clock built around one of the big red [emergency brakes](https://www.google.com/search?tbm=isch&q=notbremse%20zug&tbs=imgo:1) found in trains.

To display the time, a vintage [VFD](https://en.wikipedia.org/wiki/Vacuum_fluorescent_display) is used.

An STM32F103 microcontroller and its RTC unit is used for tracking the time. It goes into deep sleep mode most of the time.

Soundfiles from an SD card can be read by the microcontroller and played through a small speaker.

Because of the VFD's power consumption, the clock can't run on battery power alone, so a 24 V power supply is used. 

In case of a power outage, a 3 V coin cell battery powers the microcontroller. In this scenario a small buzzer will be used instead of a wakeup melody.

Also there will be support for a [DCF77](https://en.wikipedia.org/wiki/DCF77) radio clock module to automatically update the time.

A toggle switch will allow the selection of two different alarms. The times are set using a rotary encoder.


## Resources
- [Useful information](http://www.vwlowen.co.uk/arduino/vfd/vfd-clock.htm) about VFDs
- [A Guide to Fundamental VFD Operation](https://www.noritake-elec.com/technology/general-technical-information/vfd-operation)
- [Instructables page](https://www.instructables.com/id/A-Simple-Driver-for-VFD-Displays/) about controlling VFDs
- [Github project](https://github.com/AndruPol/ChibiOS-WavePlayer) for an STM32 wave player with ChibiOS
- [Another Github project](https://github.com/no111u3/stm32f4d_encoder/blob/master/main.c) about using the timer's encoder mode with ChibiOS

## Components
- old emergency brake from eBay
- VFD from [pollin.de](https://www.pollin.de/p/vakuum-fluoreszenzdisplay-futaba-4-lt-46zb3-4-digit-121537) with additional parts:
  - ~~UDN6118 VFD driver IC from eBay~~ [PT6312B](https://datasheet.lcsc.com/szlcsc/Princeton-Tech-PT6312B_C110225.pdf) VFD driver IC
  - L9110 H-bridge for VFD filament
  - MPSA13 darlington transistor from [reichelt.de](https://www.reichelt.de/bipolartransistor-npn-30v-0-5a-0-625w-to-92-mpsa-13-p13111.html?&trstct=pos_0&nbc=1])
  - GL5506 light dependent resistor from eBay
- STM32F103VET6
- SD card holder from [conrad.de](https://www.conrad.de/de/p/attend-microsd-kartensockel-druck-druck-112i-tdar-r-1-st-1308329.html)
- DCF77 module from [pollin.de](https://www.pollin.de/p/dcf-empfangsmodul-dcf1-810054)
- [LM4670](http://www.ti.com/lit/ds/snas240c/snas240c.pdf) audio amplifier
- CR2032 cell as backup battery
- small buzzer
- toggle switch from [pollin.de](https://www.pollin.de/p/kippschalter-kn3-c-103a-1-polig-on-off-on-420023)
- rotary encoder from [reichelt.de](https://www.reichelt.de/drehimpulsegeber-15-impulse-30-rastungen-vertikal-stec11b03-p73913.html?r=1)
- 24 V to 3.3 V step down regulator from [reichelt.de](https://www.reichelt.de/dc-dc-wandler-r78e-3-3-v-500-ma-sil-3-single-r-78e33-05-p177267.html?&trstct=pos_4&nbc=1) or [conrad.de](https://www.conrad.de/de/p/gaptec-lme78-03-1-0-dc-dc-wandler-print-24-v-dc-3-3-v-dc-1000-ma-3-3-w-anzahl-ausgaenge-1-x-1603783.html)

## Usage
The clock will be in deep sleep most of the time. The display can be activated for about 10 seconds by tugging the emergency brake shortly.

While the time is being displayed, flipping the toggle switch into one of its two "on" positions allows setting the alarm times. The alarms can each be de-/activated by pulling the emergency lever shortly. A deactivated alarm will be represented by \_\_:\_\_ on the display.

Setting the alarm time is done by pressing the rotary encoder's push button, then setting the hours and then pressing it again to set the minutes. Alarm times are stored inside an EEPROM IC to avoid loss of settings after a power outage.

Once one alarm rings, pulling the emergency lever shortly will put it into sleep mode, pulling longer will deactivate the alarm.
