# DNS Tunneler

This project implements a traffic tunneling method using DNS (Domain Name System) to transfer data over networks that block outgoing traffic. DNS traffic is typically allowed to pass through network restrictions, making it a suitable channel for data transmission. The tunneling technique involves encoding data into DNS packets to disguise it as regular traffic.

## Implementation

The project is organized into the following directories:

- `receiver`: Contains the receiver source code.
- `sender`: Contains the sender source code.
- `common`: Includes common components such as the base64 implementation.

The source code is written in C11 and can be compiled using the provided `Makefile`. The communication protocol between the client and server is designed to be discreet and involves a handshake process to establish the connection.

## Communication Protocol: Client-Server

The communication between the client (sender) and server (receiver) follows a three-step handshake process:

1. Hello UDP packet: The sender sends a valid DNS query packet using UDP, with the query type (`QTYPE`) set to `A` and the query name (`QNAME`) set to the `BASE_HOST` domain.
2. Hello UDP packet response: The receiver replies with a valid DNS response packet, setting the Truncated (`TC`) flag to `1`, indicating that the client should switch to using TCP instead.
3. Opening TCP connection and filename packet: The client establishes a TCP connection and sends a DNS query packet with the filename encoded in the query name (`QNAME`) along with the `BASE_HOST` suffix.
4. Server checks the file: The server attempts to open the file and closes the connection if it fails.
5. Server is ready to receive file: If the file check is successful, the server is ready to receive data packets.

## Data Encoding

Data is encoded using the base64 encoding algorithm. File chunks are encoded into alphanumeric characters (`a-zA-Z0-9+-`) with padding character `=`. The encoded data is then divided into 63-character-long labels with the `BASE_HOST` suffix, adhering to the specifications of RFC1035.

## Extensions

The implementation supports both IPv4 and IPv6 addresses. Networking is designed to support dual-stack, where IPv4 addresses are mapped to IPv6 addresses. Additionally, the receiver acts as a DNS proxy for queries that do not use the `BASE_HOST` suffix, forwarding them to a configured DNS server.

## Testing

The functionality of the receiver was tested using the `dig` command-line tool to perform DNS queries and the `diff` tool to compare the received file with the original file. Performance testing can be done by measuring the throughput of the tunnel using the provided macro in the sender code.

### Functional Testing

The receiver has been tested using [`dig`](https://linux.die.net/man/1/dig):

```bash
$ dig @127.0.0.1 vutbr.cz
; <<>> DiG 9.10.6 <<>> vutbr.cz
;; global options: +cmd
;; Got answer:
;; ->>HEADER<<- opcode: QUERY, status: NOERROR, id: 10491
;; flags: qr rd ra; QUERY: 1, ANSWER: 1, AUTHORITY: 2, ADDITIONAL: 5

;; OPT PSEUDOSECTION:
; EDNS: version: 0, flags:; udp: 4096
;; QUESTION SECTION:
;vutbr.cz.			IN	A

;; ANSWER SECTION:
vutbr.cz.		191	IN	A	147.229.2.90

;; AUTHORITY SECTION:
vutbr.cz.		1858	IN	NS	rhino.cis.vutbr.cz.
vutbr.cz.		1858	IN	NS	pipit.cis.vutbr.cz.

;; ADDITIONAL SECTION:
pipit.cis.vutbr.cz.	1	IN	A	77.93.219.110
pipit.cis.vutbr.cz.	1	IN	AAAA	2a01:430:120::4d5d:db6e
rhino.cis.vutbr.cz.	65	IN	A	147.229.3.10
rhino.cis.vutbr.cz.	65	IN	AAAA	2001:67c:1220:e000::93e5:30a

;; Query time: 54 msec
;; SERVER: 127.0.0.1#53(127.0.0.1)
;; WHEN: Thu Nov 10 23:25:16 CET 2022
;; MSG SIZE  rcvd: 185
```

#### File transer
```
sender                                          receiver
...                                             $ ./dns_receiver dobron.sk tmp
$ ./dns_sender -u ::1 dobron.sk file bin        ...
...                                             ...
[CMPL] bin of 104254B                           [CMPL] tmp/file of 69240B^C
$                                               $ diff bin tmp/file
                                                $ echo $?
                                                0
```

