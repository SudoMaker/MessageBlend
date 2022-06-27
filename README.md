# MessageBlend
Blend your messages into a stream!

## Description
**MessageBlend** is a library for sending/receiving packetized messages over stream-oriented links.

Each message can be tagged with a 16-bit "channel" number, and message boundaries are well-preserved.

It's implemented using the COBS algorithm, so it has a predictable overhead ratio, and resistant to link errors. The COBS implementation is available as a separate library.

It works with any stream-oriented digital data transportation with byte as the minimal data unit, such as serial links (UART/RS232), TCP connections, etc.

## Use Cases
- RPC over UART
- UDP over TCP
- ...

## Usage
TODO

## Example
See `test/linux_pty.c`. It demonstrates 16:1 multiplexing and 1:16 de-multiplexing using ptys on Linux.

#### Usage

Run `./test_linux_pty m` in terminal 1.

Run `./test_linux_pty s <[0] path in terminal 1 output>` in terminal 2.

Run `screen <[1] path in terminal 1 output>` in terminal 3.

Run `screen <[1] path in terminal 2 output>` in terminal 4.

Type in terminal 3 or 4 and see what happens!
