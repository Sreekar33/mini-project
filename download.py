import socket
import threading
import time
import os
import re
import urlparse
import thread
import sys
import hashlib
import matplotlib.pyplot as plt
import pylab

x=[]
y=[]
dict={}
site=str(sys.argv[1])
N=int(sys.argv[2])
url=urlparse.urlparse(site)
HOST=url.netloc
PORT=80
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST,PORT))
string="GET "+url.path+ " HTTP/1.1\nHOST: "+HOST + "\r\n\r\n"
s.send(string)
data = s.recv(1024)
data=data.replace('\r','')
data1=data.split('\n')
checksum=0
length=0
print data
for i in range(len(data1)):
	temp=data1[i].split(': ')
	if(temp[0]=='ETag'):
		colon=temp[1].replace('"','')
		checksum=colon
	if(temp[0]=='Content-Length') :
		length=int(temp[1])
		break
		
print "total length : " + str(length)
#print checksum
lock=threading.Lock()

def parts(i,st,fi):
	fil="part"+str(i)
	f=open(fil,'wb')
	s=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect((HOST,PORT))
	string="GET "+url.path+ " HTTP/1.1\nHOST: "+HOST+"\nRange: bytes="+ str(st)+"-"+str(fi)+ "\r\n\r\n"
	start=time.time()
	s.send(string)
	data=s.recv((int(fi)-int(st))+1000,socket.MSG_WAITALL)
	end=time.time()
	data=data.partition('\r\n\r\n')
	f.write(data[2])
	f.close()
	throughput=(float(fi-st)*8)/(float(end-start))
	global x
	global y
	lock.acquire()
	x=x+[throughput]
	y=y+[i+1]
	print "instantaneous throughput in chunk " + str(i) +" is "+str(throughput) +" bps "
	lock.release()


if(N>1):
	block_size=int(length/(N))
else :
	block_size=length-1
	
#print block_size

initial=0
final=0
thread={}
start=time.time()
for i in range(N-1) :
	final=int(initial)+block_size-1
	print "thread "+ str(i+1) + " downloading "+str(initial) +" to "+ str(final) + " bytes."
	thread[i]=threading.Thread(target=parts,args=(i,initial,final))
	thread[i].start()
	initial = int(final)+1

print "thread "+ str(N) + " downloading "+str(initial) +" to "+ str(length-1) + " bytes."

thread[N-1]=threading.Thread(target=parts,args=(N-1,initial,length-1))

thread[N-1].start()


for i in range(N) :
	thread[i].join()

end=time.time()
print "total time to download the file : " + str(end-start) + " seconds."
filename=[]
for i in range(N) :
	fil="part"+str(i)
	filename=filename+[fil]

file_name=str(site).split('/')[-1]
with open(file_name,'w') as outfile :
	for fname in filename :
		with open(fname) as infile :
			for line in infile:
				outfile.write(line)

print "checksum of the file : "+(hashlib.md5(open(file_name, 'r').read()).hexdigest())
plt.plot(y,x,marker='o', linestyle='--', color='r', label='Square')
plt.xlabel("thread")
plt.ylabel("throughput(bps)")
pylab.savefig('throughput.png')
for fname in filename :
	os.remove(fname)
