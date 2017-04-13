import os
import socket
import hashlib
import cPickle as pickle
import sys 
import time

PORT_NUMBER = 5003
SIZE = 4096
ACK_SIZE=1024
file_name=str(sys.argv[1])
N=int(sys.argv[2])
buf=1000
hostName = socket.gethostbyname( '0.0.0.0' )
#print hostName
mySocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

mySocket.bind((hostName, PORT_NUMBER))
print ("Test server listening on port {0}\n".format(PORT_NUMBER))
initial_timeout=2;
data,addr = mySocket.recvfrom(SIZE)
#print addr
#print data

data=file_name
mySocket.sendto(data,addr)

data,addr=mySocket.recvfrom(SIZE)
f=open(file_name,"rb") 

'''data = f.read(buf)'''

#print "sending file"
base = 0

def send_packet(sequence_no,data,address) :
	lis=[sequence_no,data,hashlib.md5(data).hexdigest()]
	arr=pickle.dumps(lis)
	mySocket.sendto(arr,address)

def recv_packet(sequence):
	mySocket.settimeout(2)
	try:
		print "try-catch for sequence no : "+ str(sequence)
		data,addr=mySocket.recvfrom(ACK_SIZE)
		arr=pickle.loads(data)
		if(arr[0]=="ACK") :
			print "ACK received sequence no: "+str(arr[1])
			return "ACK",arr[1]
		elif arr[0]=="cor":
			return "cor",arr[1]
		elif(arr[0]=="wrg"):
			return "wrg",arr[1]
	except socket.timeout:
		print "exception"
		print "packet timeout sequence no: "+str(sequence)
		return "trc",sequence

base=0
nextseqno=0
fin=0
datacount=0
data=[]
while True:
	print "base value : "+str(base)
#	print "data count : "+str(datacount)
#	print "length of data: "+str(len(data))
	if(fin==1):
		if(len(data)<N and len(data)>0):
			N=len(data)
		else:
			print "finish"
			mySocket.sendto("fin",addr);
			break;
	if len(data)==0:
		for i in range(N) :
			read_data=f.read(buf)
			if (len(read_data) ==0) :
				N=len(data)
				fin=1
				print datacount 
				#time.sleep(5)
				break
			else:
				data=data+[read_data]
				datacount=datacount + 1
#	print "length of data: "+str(len(data))		
	for i in range(N):
		print "packet sent with sequenceno : "+ str(nextseqno)
		send_packet(nextseqno,data[i],addr)
		nextseqno=nextseqno+1

	tbase=base
	for i in range(N):
		print "waiting for ack of packet with seq no : " +str(base+i)
		ret,seq=recv_packet(base+i)
		if ret=="trc":
			print "time out"
			tbase=base
		elif ret=="cor":
			print "corrupt"
			tbase=base
		elif ret=="wrg":
			print "wrong packet"
			tbase=base
		elif ret=="ACK":
			print "goinf to ack:"
			tbase=seq+1
				
	print "tbase: "+str(tbase)
	print "base :" +str(base)
	if(tbase==base+N) :
		base=base+N
		data=[]	
	nextseqno=base
	#time.sleep(1)
	#time.sleep(5)	31 32 33 34   ----  30 33 base 31 ret 33 
	#IITH 2.23
	#GUEST 167.218	
