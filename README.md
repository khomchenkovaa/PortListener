# PortListener

Listen the TCP/UDP ports and Sockets and print received data.

The PortListener boasts the following features:

 * any number of TCP/UDP/Socket listeners
 * display payloads as text or binary data
 * make reply (no reply, echo, predefined string)

The PortListener contains:
 * desktop application
 * win-service / linux daemon

To send an UDP packet to port:
```sh
echo <packetContent> | socat -t 5 - udp:<dstIp>:<dstPort>
```
This sends the packet and prints any packets coming back from there;
if none arrive, it quits after 5 seconds (-t 5).

To send an TCP packet to port:
```sh
echo <packetContent> | socat -t 5 - tcp:<dstIp>:<dstPort>
```

To send any packet to socket:
```sh
echo <packetContent> | socat -t 5 - UNIX-CONNECT:/path/to/any.sock
```

UDP packet csv-to-db example:
```sh
echo 'counters:"KBB20CQ001_D11";"1.0";"266466.2";"133233.1";"266466.0";"2023-04-24-13:45:54:222"' | socat -t 5 - udp:localhost:2424
```