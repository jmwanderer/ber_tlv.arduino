# BER TLV Encoder / Decoder

A C++ implementation for BER TLV encoding and decoding for the Arduino.

Intended to be light weight with reduced dynamic memory allocation.

Supports 1 or 2 byte tag values, definite length form, and
a maximum of 65535 TLV length.

## Using TLV

Encoding:
```
TLVS tlvs;
uint8_t buffer[255];
uint8_t abc[] = { 0x65, 0x66, 0x67 };
tlvs.addTLV(0x8a, abc, sizeof(abc));
size_t data_size = tlvs.encodeTLVs(buffer, sizeof(buffer));
```

Decoding:
```
TLVS tlvs;
TLVNode *tlvNode;
uint8_t buffer[] = { 0x9F, 0x10, 0x01, 0x31 };
tlvs.decodeTLVs(buffer, sizeof(buffer));
tlvNode = tlvs.firstTLV();
printf("Tag: %x\n", tlvNode->getTag());
```

- See tlv.ino for example usage.
- See tlv.h for full interface.


## Run the example

The example code, tlv.ino, outputs results to the serial port
at 9600 baud. 

The fastest way to run the example is to copy the src (tlv.h, tlv.cc) and 
example (tlv.ino) files to:
- Arduino IDE: into a new sketch
- Platform IO: into the src/ directory of a new proejct 

## Use as a library

This project can be used as a library with the ArduinoIDE or Platform IO.

### Arduino IDE

Select install in the library manager

### Platform IO

Set lib_deps in platformio.ini
```
lib_deps =
    https://github.com/jmwanderer/ber-tlv.arduino
```

## BER TLV Spec

BER TLV on Wikipedia
<https://en.wikipedia.org/wiki/X.690#BER_encoding>



## Note

This library is a port of https://github.com/jmwanderer/ber-tlv

Original development and testing is done on Linux and ported to the
Arduino framework.
