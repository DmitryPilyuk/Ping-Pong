# UDP Ping Pong

This project demonstrates a simple UDP-based "ping-pong" mechanism between two processes.

## Build

To build the project, navigate to the project root directory and run `make`:

```bash
make
```

This will compile the source files and create an executable named `ping_pong`.

## Run

To run the compiled program, execute the `ping_pong` executable:

```bash
./ping_pong
```

The program will create two processes that communicate using Unix domain datagram sockets, simulating a ping-pong exchange.
