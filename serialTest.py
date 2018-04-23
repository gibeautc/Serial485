#!/usr/bin/env python

import serial
import time

dumbData=[]
for x in range(100):
	dumbData.append(str(x+50).encode())
try:
	ser=serial.Serial('/dev/ttymxc2',9600,timeout=1)
except:
	print("Error Opening Serial")
	exit()

def processMessage(line):
	print("processing message:"+str(len(line)))
	errors=0
	sender=line[0]
	msgNum=line[1]
	msgType=line[2]
	data=line[3:]
	cnt=0
	#check data for errors
	for d in data:
		if d!=dumbData[cnt]:
			errors=errors+1
		cnt=cnt+1
		if cnt==len(dumbData)-1:
			errors=errors+len(data)-len(dumbData)
			break
	#now confirm the message
	out=bytearray()
	out.append(sender)
	out.append(msgNum)
	out.append(2)
	for d in data:
		out.append(d)
	for d in out:
		print(d),
	print("")
	print("Sending Confirm to device: "+str(ord(sender))+"   Type: "+str(ord(msgType))+"  MsgNum: "+str(ord(msgNum))+"   Data Errors: "+str(errors))
	for c in out:
		ser.write(c)
	ser.write('\n')
	#out=[]
	#out.append(sender)
	#out.append(msgNum)
	#out.append(0x02)
	#for d in data:
	#	out.append(d)
	#print("Out Length: "),
	#print(len(out))
	#bout=bytearray(out)
	#print(bout)
	#print("Sending Confirm to device: "+str(ord(sender))+"   Type: "+str(ord(msgType))+"  MsgNum: "+str(ord(msgNum))+"   Data Errors: "+str(errors))
	#send the confirm
	#for c in out:
	#	ser.write(c)
	#ser.write('\n')

def main():
	rxBuf=[]
	while True:
		try:
			char=ser.read()
			
		except:
			print("failed to read")
			continue
			
		if char!="\n":
			if char!="":
				rxBuf.append(char)
		else:
			processMessage(rxBuf)
			rxBuf=[]
			

if __name__=="__main__":
	main()
