# PortListener

Listen the TCP/UDP ports and Sockets and do something with received data.

The PortListener contains:
 * desktop application
 * win-service / linux daemon

## PortListener Desktop version (PLDesk)

The PortListener boasts the following features:

 * any number of TCP/UDP/Socket listeners
 * display payloads as text or binary data
 * make reply (no reply, echo, predefined string)

## PortListener Service version (PLService)

Usage:
```sh
   ./PLService [Command line arguments] [Port]
```

Command line arguments:
 * -i (-install) Install the service.
 * -u (-uninstall) Uninstall the service.
 * -e (-exec) Execute the service as a standalone application (useful for debug purposes). This is a blocking call, the service will be executed like a normal application. In this mode you will not be able to communicate with the service from the contoller.
 * -t (-terminate) Stop the service.
 * -p (-pause) Pause the service.
 * -r (-resume) Resume a paused service.
 * -c cmd (-command cmd) Send the user defined command code cmd to the service application.
 * -v (-version) Display version and status information.

Without arguments start the service

## How to send packet to port

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