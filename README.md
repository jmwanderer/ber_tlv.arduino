# BER TLV Encoder / Decoder

A C++ implementation for BER TLV encoding and decoding.

Intended to be light weight with reduced dynamic
memory allocation.

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

- See example.cc for more details.
- See tlv.h for full interface.


## Building

Compile and link with tlv.cc
```
g++ -o example example.cc tlv.cc
```

## BER TLV Spec

BER TLV on Wikipedia
<https://en.wikipedia.org/wiki/X.690#BER_encoding>

## Test Cases

run via `make tests`

Uses some test cases found in
<https://github.com/toumilov/python-ber-tlv/blob/main/tests/test.py>



