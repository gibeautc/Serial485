#!/usr/bin/env python

import serial
import time
import json
try:
	#ser=serial.Serial('/dev/ttymxc2',9600,timeout=1)
	ser=serial.Serial('/dev/ttyUSB0',4800,timeout=1)
except:
	print("Error Opening Serial")
	exit()



def confirm(dev,num):
	j=json.loads('{}')
	j['device']=dev
	j['msgnum']=num
	j['status']="confirm"
	out=json.dumps(j)
	
	outBA=bytearray()
	outBA.append(0x02)
	for c in out:
		outBA.append(c)
	outBA.append(0x03)
	ser.write(outBA)
	#ser.write(0x02)
	#for s in out:
	#	ser.write(s)
	#ser.write(0x03)
	print("Confirm Sent")

class Device():
	def __init__(self):
		self.ID=None
		self.msgNum=0
		self.dup=0
		self.status=None

class DeviceList():
	def __init__(self):
		self.idList=[]
		self.deviceList=[]
	def addDevice(self,num):
		self.idList.append(num)
		tmp=Device()
		tmp.ID=num
		self.deviceList.append(tmp)
		return
	def logMsg(self,msg):
		if msg['device'] not in self.idList:
			self.addDevice(msg['device'])
		for d in self.deviceList:
			if d.ID==msg['device']:
				if msg['msgnum']==d.msgNum:
					d.dup=d.dup+1
				d.msgNum=msg['msgnum']
				d.status=msg['status']
				confirm(d.ID,d.msgNum)
				return
			print("Cant Find it....?")
	def devPrint(self):
		for d in self.deviceList:
			print("****************")
			print("Device ID: "),
			print(d.ID)
			print("Status :   "),
			print(d.status)
			print("Msg Num:   "),
			print(d.msgNum)
			print("# dups:    "),
			print(d.dup)
		

def processMsg(msg):
	global devList
	try:
		msgStr=''.join(msg)
	except:
		print("String conversion Error")
		print msg
		return
	
	#msgStr="["+msgStr+"]"
	print(msgStr)
	print(len(msgStr))
	try:
		jsonData=json.loads(str(msgStr))
	except:
		print("Failure to parse Json")
		return
	devList.logMsg(jsonData)
	devList.devPrint()


devList=DeviceList()

def main():
	rxBuf=bytearray()
	while True:
		if ser.in_waiting==0:
			print("Waiting...")
			#give a chance for keyboard break
			time.sleep(.5)
		try:
			char=ser.read()
		except:
			print("failed to read")
			continue
		if char=="":
			continue
		if ord(char)==0x02:
			print("Start of msg")
			rxBuf=[]
		elif ord(char)==0x03:
			print("End of msg")
			processMsg(rxBuf)
		else:
			rxBuf.append(char)

if __name__=="__main__":
	main()
