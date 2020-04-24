make
stm32flash /dev/ttyUSB0 -w build/EmergencyWake.bin -S 0x8000000:50000 -v -R -b 115200