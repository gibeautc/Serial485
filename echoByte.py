#!/usr/bin/env python

import serial
import time

try:
	ser=serial.Serial('/dev/ttymxc2',9600,timeout=1)
except:
	print("Error Opening Serial")
	exit()


def main():
	while True:
		try:
			char=ser.read()
		except:
			print("failed to read")
			time.sleep(.5)
			continue
		
		if char=="":
			time.sleep(.5)
			continue	
		print("Byte Received: "),
		print(char)
		
		ser.write(char)
			

if __name__=="__main__":
	main()
