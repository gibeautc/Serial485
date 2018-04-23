#!/usr/bin/env python

import serial
import time

try:
	ser=serial.Serial('/dev/ttymxc2',9600,timeout=1)
except:
	print("Error Opening Serial")
	exit()


def main():
	lastSend=time.time()
	while True:
		for n in range(1,3):
			if n==1:
				ser.write(1)
			if n==2:
				ser.write(2)
			
			time.sleep(.1)
			resp=ser.read(1)
			if resp==n:
				print("Got good Response:"),
				print(n)
			else:
				print("Sent: "),
				print(n)
				print("Rx: "),
				print(resp)
		

if __name__=="__main__":
	main()
