EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A3 16535 11693
encoding utf-8
Sheet 1 1
Title ""
Date "2019-06-22"
Rev ""
Comp "Turag e. V."
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Sensor_Optical:LDR07 R1
U 1 1 5E28C8FB
P 1100 2650
F 0 "R1" H 1170 2696 50  0000 L CNN
F 1 "LDR07" H 1170 2605 50  0000 L CNN
F 2 "OptoDevice:R_LDR_5.1x4.3mm_P3.4mm_Vertical" V 1275 2650 50  0001 C CNN
F 3 "http://www.tme.eu/de/Document/f2e3ad76a925811312d226c31da4cd7e/LDR07.pdf" H 1100 2600 50  0001 C CNN
	1    1100 2650
	1    0    0    -1  
$EndComp
$Comp
L VFD:FUTABA_4-LT-46ZB3 D?1
U 1 1 5E2A349C
P 13750 6550
F 0 "D?1" H 13750 7065 50  0000 C CNN
F 1 "FUTABA_4-LT-46ZB3" H 13750 6974 50  0000 C CNN
F 2 "VFD:FUTABA_4-LT-46ZB3" H 13750 5100 50  0001 C CNN
F 3 "https://www.pollin.de/productdownloads/D121537D.PDF" V 13800 7400 50  0001 C CNN
	1    13750 6550
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x04 J3
U 1 1 5E2AEC4C
P 13850 2750
F 0 "J3" H 13930 2742 50  0000 L CNN
F 1 "Conn_01x04" H 13930 2651 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Horizontal" H 13850 2750 50  0001 C CNN
F 3 "~" H 13850 2750 50  0001 C CNN
	1    13850 2750
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J4
U 1 1 5E2AFF81
P 13850 3150
F 0 "J4" H 13930 3142 50  0000 L CNN
F 1 "Conn_01x02" H 13930 3051 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Horizontal" H 13850 3150 50  0001 C CNN
F 3 "~" H 13850 3150 50  0001 C CNN
	1    13850 3150
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x03 J5
U 1 1 5E2B0951
P 13850 3550
F 0 "J5" H 13930 3592 50  0000 L CNN
F 1 "Conn_01x03" H 13930 3501 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Horizontal" H 13850 3550 50  0001 C CNN
F 3 "~" H 13850 3550 50  0001 C CNN
	1    13850 3550
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x03 J6
U 1 1 5E2B1093
P 13850 3950
F 0 "J6" H 13930 3992 50  0000 L CNN
F 1 "Conn_01x03" H 13930 3901 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Horizontal" H 13850 3950 50  0001 C CNN
F 3 "~" H 13850 3950 50  0001 C CNN
	1    13850 3950
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J7
U 1 1 5E2B13EE
P 13850 4250
F 0 "J7" H 13930 4242 50  0000 L CNN
F 1 "Conn_01x02" H 13930 4151 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Horizontal" H 13850 4250 50  0001 C CNN
F 3 "~" H 13850 4250 50  0001 C CNN
	1    13850 4250
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x04 J8
U 1 1 5E2B196C
P 13850 4650
F 0 "J8" H 13930 4642 50  0000 L CNN
F 1 "Conn_01x04" H 13930 4551 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Horizontal" H 13850 4650 50  0001 C CNN
F 3 "~" H 13850 4650 50  0001 C CNN
	1    13850 4650
	1    0    0    -1  
$EndComp
Text Notes 14500 2800 0    50   ~ 0
SWD
Text Notes 14500 3200 0    50   ~ 0
Speaker
Text Notes 14500 3550 0    50   ~ 0
DCF77
Text Notes 14500 3950 0    50   ~ 0
Toggle button
Text Notes 14500 4300 0    50   ~ 0
Lever
Text Notes 14500 4700 0    50   ~ 0
Encoder
$Comp
L Connector:Micro_SD_Card_Det J2
U 1 1 5E2B971C
P 1950 9100
F 0 "J2" H 1900 9917 50  0000 C CNN
F 1 "Micro_SD_Card_Det" H 1900 9826 50  0000 C CNN
F 2 "Card:microSD_Attend_112I-TDAR-R 1" H 4000 9800 50  0001 C CNN
F 3 "https://www.hirose.com/product/en/download_file/key_name/DM3/category/Catalog/doc_file_id/49662/?file_category_id=4&item_id=195&is_series=1" H 1950 9200 50  0001 C CNN
	1    1950 9100
	-1   0    0    -1  
$EndComp
$Comp
L Device:Battery_Cell BT1
U 1 1 5E2BD2DD
P 3650 3000
F 0 "BT1" H 3768 3096 50  0000 L CNN
F 1 "Battery_Cell" H 3768 3005 50  0000 L CNN
F 2 "Battery:BatteryHolder_Keystone_3002_1x2032" V 3650 3060 50  0001 C CNN
F 3 "~" V 3650 3060 50  0001 C CNN
	1    3650 3000
	1    0    0    -1  
$EndComp
$Comp
L Connector:Barrel_Jack_Switch J1
U 1 1 5E2BE1AB
P 1700 1750
F 0 "J1" H 1757 2067 50  0000 C CNN
F 1 "Barrel_Jack_Switch" H 1757 1976 50  0000 C CNN
F 2 "Connector_BarrelJack:BarrelJack_CUI_PJ-102AH_Horizontal" H 1750 1710 50  0001 C CNN
F 3 "~" H 1750 1710 50  0001 C CNN
	1    1700 1750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR02
U 1 1 5E2BF81A
P 2100 1950
F 0 "#PWR02" H 2100 1700 50  0001 C CNN
F 1 "GND" H 2105 1777 50  0000 C CNN
F 2 "" H 2100 1950 50  0001 C CNN
F 3 "" H 2100 1950 50  0001 C CNN
	1    2100 1950
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR05
U 1 1 5E2BFCAF
P 3650 3200
F 0 "#PWR05" H 3650 2950 50  0001 C CNN
F 1 "GND" H 3655 3027 50  0000 C CNN
F 2 "" H 3650 3200 50  0001 C CNN
F 3 "" H 3650 3200 50  0001 C CNN
	1    3650 3200
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR03
U 1 1 5E2C0277
P 3050 9750
F 0 "#PWR03" H 3050 9500 50  0001 C CNN
F 1 "GND" H 3055 9577 50  0000 C CNN
F 2 "" H 3050 9750 50  0001 C CNN
F 3 "" H 3050 9750 50  0001 C CNN
	1    3050 9750
	1    0    0    -1  
$EndComp
$Comp
L MCU_ST_STM32F1:STM32F103VETx U2
U 1 1 5E2C0F1E
P 8400 5450
F 0 "U2" H 8400 2561 50  0000 C CNN
F 1 "STM32F103VETx" H 8400 2470 50  0000 C CNN
F 2 "Package_QFP:LQFP-100_14x14mm_P0.5mm" H 7600 2850 50  0001 R CNN
F 3 "http://www.st.com/st-web-ui/static/active/en/resource/technical/document/datasheet/CD00191185.pdf" H 8400 5450 50  0001 C CNN
	1    8400 5450
	1    0    0    -1  
$EndComp
$Comp
L Regulator_Switching:R-78C3.3-1.0 U1
U 1 1 5E2C31A6
P 3200 2000
F 0 "U1" H 3200 2242 50  0000 C CNN
F 1 "R-78C3.3-1.0" H 3200 2151 50  0000 C CNN
F 2 "Converter_DCDC:Converter_DCDC_RECOM_R-78E-0.5_THT" H 3250 1750 50  0001 L CIN
F 3 "https://www.recom-power.com/pdf/Innoline/R-78Cxx-1.0.pdf" H 3200 2000 50  0001 C CNN
	1    3200 2000
	1    0    0    -1  
$EndComp
$Comp
L Device:R_POT RV1
U 1 1 5E2C4A43
P 1100 3100
F 0 "RV1" H 1031 3146 50  0000 R CNN
F 1 "R_POT" H 1031 3055 50  0000 R CNN
F 2 "" H 1100 3100 50  0001 C CNN
F 3 "~" H 1100 3100 50  0001 C CNN
	1    1100 3100
	1    0    0    -1  
$EndComp
$Comp
L Transistor_BJT:MPSA42 Q1
U 1 1 5E2C5B65
P 1450 2900
F 0 "Q1" H 1641 2946 50  0000 L CNN
F 1 "MPSA13" H 1641 2855 50  0000 L CNN
F 2 "Package_TO_SOT_THT:TO-92_Inline" H 1650 2825 50  0001 L CIN
F 3 "http://www.onsemi.com/pub_link/Collateral/MPSA42-D.PDF" H 1450 2900 50  0001 L CNN
	1    1450 2900
	1    0    0    -1  
$EndComp
$Comp
L Device:Buzzer BZ1
U 1 1 5E2C6BE1
P 13650 1500
F 0 "BZ1" H 13802 1529 50  0000 L CNN
F 1 "Buzzer" H 13802 1438 50  0000 L CNN
F 2 "Buzzer_Beeper:Buzzer_12x9.5RM7.6" V 13625 1600 50  0001 C CNN
F 3 "~" V 13625 1600 50  0001 C CNN
	1    13650 1500
	1    0    0    -1  
$EndComp
$Comp
L Device:Crystal Y1
U 1 1 5E2C737B
P 10100 6150
F 0 "Y1" H 10100 6418 50  0000 C CNN
F 1 "Crystal" H 10100 6327 50  0000 C CNN
F 2 "" H 10100 6150 50  0001 C CNN
F 3 "~" H 10100 6150 50  0001 C CNN
	1    10100 6150
	1    0    0    -1  
$EndComp
$Comp
L Device:C C1
U 1 1 5E2C7D9C
P 10150 6600
F 0 "C1" H 10265 6646 50  0000 L CNN
F 1 "C" H 10265 6555 50  0000 L CNN
F 2 "" H 10188 6450 50  0001 C CNN
F 3 "~" H 10150 6600 50  0001 C CNN
	1    10150 6600
	1    0    0    -1  
$EndComp
$Comp
L Device:C C2
U 1 1 5E2C87A9
P 10450 6600
F 0 "C2" H 10565 6646 50  0000 L CNN
F 1 "C" H 10565 6555 50  0000 L CNN
F 2 "" H 10488 6450 50  0001 C CNN
F 3 "~" H 10450 6600 50  0001 C CNN
	1    10450 6600
	1    0    0    -1  
$EndComp
$Comp
L Device:D_Schottky D1
U 1 1 5E2D4271
P 3850 2500
F 0 "D1" H 3850 2716 50  0000 C CNN
F 1 "D_Schottky" H 3850 2625 50  0000 C CNN
F 2 "" H 3850 2500 50  0001 C CNN
F 3 "~" H 3850 2500 50  0001 C CNN
	1    3850 2500
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR04
U 1 1 5E2D57EC
P 3200 2450
F 0 "#PWR04" H 3200 2200 50  0001 C CNN
F 1 "GND" H 3205 2277 50  0000 C CNN
F 2 "" H 3200 2450 50  0001 C CNN
F 3 "" H 3200 2450 50  0001 C CNN
	1    3200 2450
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR06
U 1 1 5E2D5C19
P 13500 1600
F 0 "#PWR06" H 13500 1350 50  0001 C CNN
F 1 "GND" H 13505 1427 50  0000 C CNN
F 2 "" H 13500 1600 50  0001 C CNN
F 3 "" H 13500 1600 50  0001 C CNN
	1    13500 1600
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR01
U 1 1 5E2D6D50
P 1100 3400
F 0 "#PWR01" H 1100 3150 50  0001 C CNN
F 1 "GND" H 1105 3227 50  0000 C CNN
F 2 "" H 1100 3400 50  0001 C CNN
F 3 "" H 1100 3400 50  0001 C CNN
	1    1100 3400
	1    0    0    -1  
$EndComp
$EndSCHEMATC
