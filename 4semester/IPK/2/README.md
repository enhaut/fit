# IPK project 2 - Packet sniffer
## Assigment
Implement packet sniffer that should support sniffing IPv4, IPv6, ICMP, ICMPv6 and also ARP.

## Usage 
Compile project using `Makefile`:
```bash
$ make
```
Then, it could be started with:
```bash
$ ./ipk-sniffer [-i interface | --interface interface] {-p port} {[--tcp|-t] [--udp|-u] [--arp] [--icmp] } {-n num}
```
There is a limitation, `--port|-p` argument could not be provided together with `--arp` or `--icmp`,
also it does not make sense to filter packets by port for protocols that does not use ports at all.

## Output
The output using command `./ipk-sniffer -i eno1 --port 50 --udp` looks like:
```bash
timestamp: 2022-04-24T08:04:28.763121+01:00
src MAC: 20:4e:71:ff:ff:ff
dst MAC: 70:b5:e8:ff:ff:ff
frame length: 60 bytes
src IP: 10.0.0.15
dst IP: 10.0.0.1
src port: 64043
dst port: 50

0x0000: 45 00 00 22 43 c2 00 00 30 11 f2 42 0a 28 c0 0f E.."C...0..B.(..
0x0010: 0a 13 80 7c ef 56 00 32 00 0e 77 a6 68 65 6c 6c ...|.V.2..w.hell
0x0020: 6f 0a 00 00 00 00 00 00 00 00 53 4d c5 ce 00 00 o.........SM....
0x0030: 00 00 00 00 00 00 00 00 00 00 00 00 ............
```
Columns (position, hex values and text representation) are separated by `\t` character.
For more information see [complete documentation](manual.pdf).

## Author
Samuel Dobron (xdobro23)  
Faculty of Information Technology, Brno University of Technology
