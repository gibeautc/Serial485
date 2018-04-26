#!/usr/bin/env python
#sudo chmod a+rw /dev/ttyUSB1
import serial
import time
import json

#need to decide what the system is, and what port to use

try:
	#ser=serial.Serial('/dev/ttymxc2',9600,timeout=1)
	ser=serial.Serial('/dev/ttyUSB0',38400,timeout=1)
except:
	print("Error Opening Serial")
	exit()




class Device():
	def __init__(self):
		self.ID=None
		self.status=None
		self.lastContact=time.time()
	def devPrint(self):
		print("****************")
		print("Device ID: "),
		print(self.ID)
		print("Status :   "),
		print(self.status)
		print("Last Contact: "),
		print(time.time()-self.lastContact)
					
def sendMsg(msg):
	out=json.dumps(msg)
	outBA=bytearray()
	outBA.append(0x02)
	for c in out:
		outBA.append(c)
	outBA.append(0x03)
	ser.write(outBA)
	
def recMsg(timeout):
	rxBuf=bytearray()
	t=time.time()
	#receive message from bus, return it, or None
	while time.time()-t<timeout:
		try:
			char=ser.read()
		except:
			print("failed to read")
			continue
		if char=="":
			continue
		if ord(char)==0x02:
			rxBuf=[]
		elif ord(char)==0x03:
			try:
				msgStr=''.join(rxBuf)
			except:
				print("String conversion Error")
				print msg
				return None
			#print(msgStr)
			#print(len(msgStr))
			try:
				jsonData=json.loads(str(msgStr))
				return jsonData
			except:
				print("Failure to parse Json")
				return None
		else:
			rxBuf.append(char)
	return None
	
			
		
def checkClients(devices):
	msg=json.loads("{}")
	msg['type']="status"
	for d in devices:
		msg['id']=d.ID
		sendMsg(msg)
		#had a delay here, but seems it is not needed
		conf=recMsg(5)
		if conf is None:
			print("recMsg returned none")
			d.status="Lost Contact"
			continue
		#print(conf)
		d.status=conf['status']
		d.lastContact=time.time()
	
def checkNew():
	msg=recMsg(1)
	if msg is None:
		return None
	try:
		print("message from new Client: "),
		print(msg['id'])
		#probably want another check here for type of "init" or something
		#then return the new id
		return msg['id']
	except:
		print("Got message, but cant read ID from it?")
		return None
	
	

def main():
	devices=[]
	
	while True:
		
		print("\n")
		for d in devices:
			d.devPrint()
		print("\n")
		s=time.time()
		checkClients(devices)
		s=time.time()-s
		print("Status Check Took: "),
		print(s),
		print(" seconds")
		time.sleep(5)
		n=checkNew()
		if n is not None:
			dup=False
			#check if its already in list
			for d in devices:
				if d.ID==n:
					print("Already Have this ID: "),
					print(d)
					dup=True
			if not dup:
				tmp=Device()
				tmp.ID=n
				devices.append(tmp)

if __name__=="__main__":
	main()
