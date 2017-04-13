import time
import sys
from socket import *
import os
import cPickle as pickle
import hashlib
SERVER_IP   = str(sys.argv[1])
PORT_NUMBER = 5003
SIZE = 4096
buf=1000
mySocket = socket( AF_INET, SOCK_DGRAM )
x="connection request"
addr=(SERVER_IP,PORT_NUMBER)
mySocket.sendto(x,addr)

data,addr=mySocket.recvfrom(SIZE)
x="rec"
mySocket.sendto(x,addr)
print data
f=open(data,"wb")
#file_size=int(x[1])
buf=1000
var=-1
expvar=0
fdata=[]
while True :
	data,addr=mySocket.recvfrom(SIZE)
	print "expected seqno : "+str(expvar)
	if(data=="fin") :
		print "finish"
		break
	else :
		arr=pickle.loads(data)
		if arr[0]==expvar :
			checksum=hashlib.md5(arr[1]).hexdigest()
			if checksum==arr[2]:
				f.write(arr[1])
				print "packet received with sequence no "+str(expvar)
				ack=["ACK",expvar]
				str_ack=pickle.dumps(ack)
				print "sending Ack with sequence no "+ str(expvar)
				mySocket.sendto(str_ack,addr)
				var=var+1
				expvar=expvar+1
			else :
				print "corrupted"
				print "sending Ack with sequence no "+ str(var)
				ack=["ACK",var]
				str_ack=pickle.dumps(ack)
				mySocket.sendto(str_ack,addr)
		else :
			print "wrong seqno received"
			print "expected seqno : "+str(expvar)+" but received : "+str(arr[0])
			print "sending Ack with sequence no "+ str(var)
			ack=["ACK",var]
			str_ack=pickle.dumps(ack)
			mySocket.sendto(str_ack,addr)
			
