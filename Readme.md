# IPK project 1 - lightweight HTTP server
## Assignment
Implement lightweight HTTP server in C/C++ with minimum requirements.
Create `Makefile` to build application. Server should be started with
```bash
$ ./hinfosvc [PORT]
```
And also should support killing by Ctrl+C.  
These endpoints are required:
1. `GET http://servername:[PORT]/hostname` - returns hostname of machine
2. `GET http://servername:[PORT]/cpu-name` - returns CPU used by machine
3. `GET http://servername:[PORT]/load` - returns percentage CPU usage

## Usage
### Compilation
Firstly, the server needs to be compiled. Just use 
provided `Makefile` and compile program with:
```bash
$ make
```

### Run
After successfully compilation, server could be started
with following command:
```bash
$ ./hinfosvc [PORT]
```
Don't forget to replace `[PORT]` with port you want the server
to listen on. For example:
```bash
$ ./hinfosvc 12345
```
Server should listen and accept connections at `[PORT]`
after it prints `Listening on [PORT]`.

### Available endpoints
Server supports 3 endpoints, these could be reached using
standard web browser or tools like 
[wget](https://www.gnu.org/software/wget/) or [curl](https://curl.se/).
All the examples bellow assumes, that you have server already running.  
_Personally I prefer to use `curl`, so these examples uses it._
#### /hostname
Server returns hostname of machine it runs on.  
Program uses [`gethostname()`](https://man7.org/linux/man-pages/man2/gethostname.2.html)
function to detect actual hostname.
```bash
$ curl http://localhost:[PORT]/hostname
merlin.fit.vutbr.cz
```
#### /cpu-name
Server returns name of first CPU found in `/proc/cpuinfo`.
```bash
$ curl http://localhost:[PORT]/cpu-name
Intel(R) Xeon(R) Silver 4214R CPU @ 2.40GHz
```
#### /load
Server returns CPU usage, calculation is inspired by 
[this](https://stackoverflow.com/a/23376195) algorithm.
```bash
$ curl http://localhost:[PORT]/load
7%
```

## Implementation details
Lightweight HTTP server is implemented as modular program,
that means you can easily add new endpoints without any refactor.  
Server does not support HTTPS nor IPv6.
  
The entrypoint of program is function `main()` in `main.c` file, 
that function is responsible for starting server, listening at `[PORT]`
and also for accepting new connections.  
  
Connection as file descriptor are then passed to `process_connection()` 
function also located in `main.c` file.  
`process_connection()` is responsible to read connection body, in our 
case the HTTP header and body. Project does not use the body at all,
so only `BUFFER_SIZE` bytes are read, the rest is just "collected".
  
Based on the requested endpoint from connection, corresponding 
endpoint is found in `ENDPOINTS` array (located in `endpoints.c` file).
If you want to add a new endpoint, just add it there. Registered function
(2nd member of struct `endpoint_t`) should be pointer to the function
that "handles" request. It should return pointer to the `response_t` struct
that contains corresponding HTTP status code (`response.h/statuses`) and
string to be sent back to the user.
  
The response is then send back to the sender using function `send_response()`
located in `main.c` file. Firstly it sends the headers defined 
as a macro `RESPONSE_HEADER` in `response.h` file, and then it sends
the string from `content` (member of `response_t` struct defined in `response.h`).
  
Connection is then closed.  
Anytime during this process could be server killed by `SIGINT` signal,
signal handler (`signal_handler()` located in `main.c`) releases
all the allocated resources and also closes all the connections.

## Author
Samuel Dobron (xdobro23)  
Faculty of Information Technology, Brno University of Technology
