import socket
import cv2
import numpy as np

import threading
import time
#print ("1-144p 2-240p 3-360p 4-480p 5-720p")
#x=input("enter the choice : ")
#i=int(x)
i=1
host = socket.gethostname()
port =int(input("enter port number : "))
size = 1024
#144p  256*144 done 1
#240p  320*240 done 2
#360p  480*360 done 3
#480p  640×480 done 4
#720p  1280*720 done 5

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print ("listening on port : " + str(port) + "addr "+str(host))
def fun2():
        while 1:
                x=input()
                global i
                i=int(x)

f=0
#threading.Thread(target=fun2).start()
vc=cv2.VideoCapture('../test.mp4')
print ("file opened")

length = int(vc.get(cv2.CAP_PROP_FRAME_COUNT))
width  = int(vc.get(cv2.CAP_PROP_FRAME_WIDTH))
height = int(vc.get(cv2.CAP_PROP_FRAME_HEIGHT))
information_passed=0
print (str(length))
s.bind((host,port))
s.listen(5)
client, address = s.accept()
print (client)
print (address)
data=client.recv(1024).decode()
if (data == "info") :
        st=str(length)
        client.send(st.encode())
else :
        vc.set(1,int(data))


while (vc.isOpened()):
    rval, frame = vc.read()
    if rval:
        #hsdhku
        #frame=numpy.tostring(frame)
        if i==1 :
                res=(256,144)
        elif i==2 :
                res=(320,240)
        elif i==3 :
                res=(480,360)
        elif i==4 :
                res=(640,480)
        elif i==5 :
                res=(1280,720)

        thumb = cv2.resize(frame, res, interpolation = cv2.INTER_AREA)
        di=np.shape(thumb)
        #print (di)
        x=np.array(thumb)
        y=x.tobytes()
        #print (len(y))
        client.send(y)
        f=f+1
        client.settimeout(1)
        while True:
                try:
                        data=client.recv(1024).decode()
                        break;
                except timeout:
                        print ("sending again")
                        client.send(y)
        x=data.split(' ')
        change=0
        if (data=="rec"):
                rec=0
        elif (data=="wrong"):
                cond=0
                while (cond==0):
                        print("retransmitting frame "+ str(f))
                        while True:
                                client.send(y)
                                try:
                                        data=client.recv(1024).decode()
                                        break
                                except timeout:
                                         print ("sending again")

                        if(data=="rec"):
                                cond=1
        elif (data=="close"):
                break;
        elif(len(x)==2):
                i=int(x[1])
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

vc.release()
cv2.destroyAllWindows()
client.close()
                                
