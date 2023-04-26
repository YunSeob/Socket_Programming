<!-- Header Start -->
<div align="center">
  <img src="https://capsule-render.vercel.app/api?type=waving&color=0:D4E6F1,100:3498DB&height=200&fontColor=FFFFFF&section=header&text=Develop&fontSize=80" />
  <div align="center"> 
    <h3>:round_pushpin: Tech</h3>
    <img src="https://img.shields.io/badge/C-A8B9CC?style=flat&logo=C&logoColor=white"/>
    <img src="https://img.shields.io/badge/Visual Studio Code-007ACC?style=flat&logo=Visual Studio Code&logoColor=white"/>
    <img src="https://img.shields.io/badge/Linux-FCC624?style=flat&logo=Linux&logoColor=black"/>
    <img src="https://img.shields.io/badge/Ubuntu20.04-E95420?style=flat&logo=Ubuntu&logoColor=white"/>
  </div>
</div>
<br />

<!-- Header End -->

<!-- Content Start -->
<h4>Project Title</h4>
✔️ Receiving video stream from IP camera using C language and Socket Programming

<br/>

<h4>Develop</h4>
<p>Implement Server</p>
- Receive packet from IP camera <br/>
- Send stream from IP camera to Client <br/>
　- Using server socket and client_port <br/>
　- client_port : It is determined in the <b>SETUP Request message</b> of the RTSP <br/>
- As soon as the stream from the IP camera is received, it is sent to the Client <br/> <br/>
 
 
<p>Implement Client</p>
- Receive packet from server <br/>
- It consists of UDP packets <br/> <br/>

<p>RTSP(Real-time Transport Streaming Protocol)</p>
- If u do packet capture with Wireshark, the packet use UDP protocol <br/>
- No matter the transport(UDP or TCP) the data format is mainly the same: <br/>
　- RTP data : [RTP Header - 12 bytes][Video stream] <br/>
　- UDP : [RTP data] <br/>
　- Hence, it received a UDP packet with an RTP header added <br/> <br/>

 
<p>Add latency</p>
- Server <br/>
　- When the Server sends a video stream packet to a Client, it writes <b>idx, bytes, time</b> in the csv file <br/>
　- Find the delay of the time before transmitting the video stream and the time when the transmission is completed <br/>
- Client <br/>
　- When the Client receives a video stream packet from a Server, it writes <b>idx, bytes, time</b> in the csv file<br/>
　- Find the delay of the time before receiving the video stream and the time when the receiving is completed<br/> <br/> <br/>
 
<p>:thinking: Future works</p>
- Server receives a video stream from one IP camera :arrow_right: Server receives multiple video streams from multiple IP cameras <br/> 
- Server uses multicast to send a video stream to the clients </br>
- Visualizes latency and bytes transmitted per second for 5s streaming <br/>
- VMAF(Video Multimethod Assessment Fusion)


<br/>

<h4>How to use</h4>

<br/>

<h4>Dependencies</h4>
✔️ C language complier such as gcc <br/>
✔️ Visual Studio Code for monitoring

<br/>

<!-- Content End -->


