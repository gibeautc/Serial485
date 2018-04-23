#!/usr/bin/env python

import serial
import time
import json

ser0=None
ser1=None

def serSetup():
	global ser0,ser1

	try:
		ser0=serial.Serial('/dev/ttyUSB0',115200,timeout=1)
	except:
		ser0=None

	try:
		ser1=serial.Serial('/dev/ttyUSB1',115200,timeout=1)
	except:
		ser1=None


rxBuf0=bytearray()
rxBuf1=bytearray()
while True:
	time.sleep(.5)
	try:
		while ser0.in_waiting:
			line=ser0.readline()
			print(line)
		
		while ser1.in_waiting:
			line=ser1.readline()
			print("\t\t\t\t\t\t\t\t\t"),
			print(line)
	except:
		serSetup()
