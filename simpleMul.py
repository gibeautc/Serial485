#!/usr/bin/env python

import serial
import time

#add check for soc or pi and setup serial accordingly



try:
	ser=serial.Serial('/dev/ttymxc2',9600,timeout=1)
except:
	print("Error Opening Serial")
	exit()


def main():
	lastSend=time.time()
	while True:
		for n in range(0,255):
			ser.write(n)
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
