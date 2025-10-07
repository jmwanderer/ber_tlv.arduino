//
// tlv.h - Simple BER TLV encoder / decoder
//
// Copyright (c) 2025 James Wanderer
//
// Encode and decode BER TLV values to / from pre-allocated buffers.
// 1 or 2 byte tags
// Definite length format - max 65532 length
// Optional malloc and copy data values.
//
//  Decode:
//  TLVS tlvs;
//  tlvs.decodeTLVs(buffer, sizeof(buffer));
//
//  Encode:
//  TLVS tlvs;
//  tlvs.addTLV(0x8a, data, sizeof(data));
//  tlv.encodeTLVs(buffer, sizeof(buffer));
//
#ifndef __TLV_H__
#define __TLV_H__

#include <stdint.h>
#include <stddef.h>


class TLVS;
class ReadBuffer;
class WriteBuffer;


//
// Represents a single TLV.
// Has a tag
// May have a data value or may have child TLVs.
//
class TLVNode {
public:
    // Access tag and value
    uint16_t getTag();
    uint32_t getValueLength();
    const uint8_t* getValue();

    // Access child TLVs
    TLVNode* firstChild();
    TLVNode* nextChild(TLVNode* child);
    TLVNode* findChild(uint16_t tag);
    
    // Utility functions to encode decode tags and length
    // Public to enable testing.
    static uint16_t parseTag(ReadBuffer &buffer, int *error);
    static int encodeTag(uint16_t tag, WriteBuffer & buffer);
    static uint16_t parseLength(ReadBuffer &buffer, int *error);
    static int encodeLength(uint32_t length, WriteBuffer &buffer);

private:
    TLVNode(uint16_t tag = 0, uint16_t length = 0);
    ~TLVNode();

    void freeContents();
    uint32_t getTotalBytes();
    void encodeTLVNode(TLVS *tlvs, WriteBuffer &buffer);
    void decodeTLVNode(TLVS *tlvs, ReadBuffer &buffer);
    void clearCachedSize();
    void addChild(TLVNode* node);

    // value may be NULL
    const uint8_t  *value;

    // May represent the length of value, or the total length of the child TLVs
    uint16_t value_length;

    // May have a value or child TLVs, but not both

    TLVNode  *parent;       // Parent TLV
    TLVNode  *next;         // Next child of parent TLV
    TLVNode  *child;        // First child of this TLV
    
    // True if value was copied and should be freed.
    bool value_allocated;

    uint16_t tag;       // support 1 or 2 bytes

    friend class TLVS;
};


//
// List of TLV values.
// Supports encode / decode. Adding TLVs
// 
class TLVS {
public:
    TLVS();

    // Free any allocates values and ready for re-use.
    void reset();

    // Encode TLV Nodes into the buffer.
    size_t encodeTLVs(uint8_t *buffer, size_t buffer_size);

    // Decode buffer contents and create TLV nodes
    void decodeTLVs(const uint8_t *buffer, size_t buffer_size);
    
    // Report first error, if any, from an encode or decode.
    // 0 == no error.
    int errorValue();

    // Access TLVs
    TLVNode* firstTLV();
    TLVNode* nextTLV(TLVNode* tlvNode);
    TLVNode* findTLV(uint16_t tag);
    TLVNode* findNextTLV(TLVNode* node);

    // *********  Add new TLVs

    // Add empty TLV
    TLVNode* addTLV(uint16_t tag);

    // Add empty TLV as a nested / child TLV
    TLVNode* addTLV(TLVNode* parent, uint16_t tag);

    // Add a TLV with a binary value.
    TLVNode* addTLV(uint16_t tag, const uint8_t *value, uint16_t value_length);

    // Add a TLV with a binary value. Allocate memory and copy the value.
    TLVNode* addTLVCopy(uint16_t tag, const uint8_t *value, uint16_t value_length);

    // Add a child / nested TLV with a binary value.
    TLVNode* addTLV(TLVNode* parent, uint16_t tag, const uint8_t *value, uint16_t value_length);

    // Add a child / nexted TLV with a binary value. Allocate memory and copy the value.
    TLVNode* addTLVCopy(TLVNode* parent, uint16_t tag, const uint8_t *value, uint16_t value_length);


    // Utility functions

    // print a binary buffer as a hex string.
    static void printHex(const uint8_t *data, size_t length);
    static void printValue(const uint8_t *data, size_t length);

    // Print a TLV to stdout
    static void printTLV(TLVNode* node, int indent=0);

    // Parse a hex string to a binary buffer
    static size_t hexToBin(const char* str, uint8_t* buffer, size_t buffer_size);

    // Maximum size of a TLV
    static const int MAX_DATA_LENGTH = (1<<16) - 1;

    // Errors
    static const int ERROR_NONE = 0;
    static const int ERROR_TAG_LENGTH = 1;
    static const int ERROR_LONG_DATA = 2;
    static const int ERROR_BAD_LENGTH = 3;
    static const int ERROR_PRIMIVE_TYPE = 4;
    static const int ERROR_END_DATA = 5;

private:
    void markError(int error);
    TLVNode* findTLVHelper(TLVNode* node, uint16_t tag);
    
    TLVNode dummy_node;     // Child TLVs are the list of TLVs
    int error_value;        // Encode / decode error, if any.
    friend class TLVNode;
};

// Utility functions to work with tags and length values
class Tag {
public:
    static uint8_t leading_byte(uint16_t tag);
    static bool tagConstructed(uint16_t tag);
    static bool tagClass(uint16_t tag);
    static uint16_t numTagBytes(uint16_t tag);
    static uint16_t numLengthBytes(uint16_t length);
};


// Interface to read buffers to avoid over-runs, etc.
// Potential for performance optimization here.
class ReadBuffer {
public:
    ReadBuffer();
    ReadBuffer(const uint8_t *buffer, uint16_t size);

    // Point to a sub-portion of an existing buffer.
    ReadBuffer(ReadBuffer buffer, uint16_t size);

    // True if end of data reached
    bool atEnd();

    // Change read position by count
    void seek(size_t count);

    // Read a byte into value. Return false if out of data
    bool getByte(uint8_t &value);

    // Return a pointer to the current read postion
    const uint8_t *position();

    // Pointer to the buffer, may be NULL
    const uint8_t *buffer;

    // Size of the total dat in the buffer
    size_t buffer_size;

    // Current read position in buffer
    size_t pos;
};

// Interface to write buffers to avoid over-runs, etc.
// Potential for performance optimization here.
class WriteBuffer {
public:
    WriteBuffer();
    WriteBuffer(uint8_t *buffer, uint16_t size);

    // Write a byte into the buffer. Return false if out of space
    bool putByte(uint8_t value);

    // Write multiple into the buffer. Return false if out of space
    bool putBytes(const uint8_t *values, uint8_t len);
    
    // Return a pointer to the current write position
    uint8_t *position();

    // Pointer to the buffer, may be NULL
    uint8_t *buffer;

    // Size of the buffer
    size_t buffer_size;

    // Position for the next write operation
    size_t pos;
};

#define TLV_TYPE_MASK 0x20      // bit 6 P/C - first byte
#define TLV_TAG_MASK 0x1F       // bit 1-5  -first byte
#define TLV_CLASS_MASK 0xC0     // bit 7 - 8 - first byte
#define TLV_LEN_MASK 0x7F       // bit 1 - 7 - len byte - 

   
#endif
