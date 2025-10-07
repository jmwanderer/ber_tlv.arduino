//
// Test cases for tlv functions
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "tlv.h"

// Test dataBuffer class
void test_databuffer()
{
    uint8_t buf[255];
    int count;
    ReadBuffer readBuf(buf, sizeof(buf));
    WriteBuffer writeBuf(buf, sizeof(buf));

    assert(!readBuf.atEnd());
    readBuf.seek(sizeof(buf));
    assert(readBuf.atEnd());

    writeBuf.pos = 0;
    writeBuf.putByte(0x1f);


    uint8_t byte;
    readBuf.pos = 0;
    readBuf.getByte(byte);
    assert(byte== 0x1f);

    for (count = 0; count < 1024; count++) {
        writeBuf.putByte(0xee);
    }
    assert(writeBuf.pos == sizeof(buf));

    writeBuf.pos = 0;
    for (count = 0; count < sizeof(buf); count++) {
        writeBuf.putByte(count);
    }
    readBuf.pos = 0;
    for (count = 0; count < 220; count++) {
        readBuf.getByte(byte);
    }
    ReadBuffer sub_buffer(readBuf, 20);
    sub_buffer.getByte(byte);
    assert(byte == 220);
    for (count = 0; count < 19; count++) {
        sub_buffer.getByte(byte);
    }
    assert(byte == 239);
    assert(sub_buffer.atEnd());
}

// Test tag functions
void test_tags()
{
    int error;
    assert(Tag::leading_byte(0x84) == 0x84);
    assert(Tag::leading_byte(0x6f29) == 0x6f);

    assert(Tag::tagConstructed(0x6f));
    assert(!Tag::tagConstructed(0x84));

    uint8_t buf[127];
    WriteBuffer writeBuf(buf, sizeof(buf));
    ReadBuffer readBuf(buf, sizeof(buf));

    uint16_t tag = 0x5f2d;
    error = TLVNode::encodeTag(tag, writeBuf);
    assert(!error);
    assert(writeBuf.pos == 2);
    assert(Tag::numTagBytes(tag) == 2);
    readBuf.pos = 0;
    assert(TLVNode::parseTag(readBuf, &error) == tag);
    assert(error == 0);

    tag = 0x6f;
    writeBuf.pos = 0;
    error = TLVNode::encodeTag(tag, writeBuf);
    assert(!error);
    assert(writeBuf.pos == 1);
    assert(Tag::numTagBytes(tag) == 1);
    readBuf.pos = 0;
    assert(TLVNode::parseTag(readBuf, &error) == tag);
    assert(error == 0);

    uint16_t val_len = 1;
    assert(Tag::numLengthBytes(val_len) == 1);
    writeBuf.pos = 0;
    error = TLVNode::encodeLength(val_len, writeBuf);
    assert(!error);
    assert(writeBuf.pos == 1);
    readBuf.pos = 0;
    assert(TLVNode::parseLength(readBuf, &error) == val_len);
    assert(error == 0);


    val_len = 1000;
    assert(Tag::numLengthBytes(val_len) == 3);
    writeBuf.pos = 0;
    error = TLVNode::encodeLength(val_len, writeBuf);
    assert(!error);
    assert(writeBuf.pos == 3);
    readBuf.pos = 0;
    assert(TLVNode::parseLength(readBuf, &error) == val_len);
    assert(error == 0);
}

const uint8_t df_name[] = 
{
    0x32, 0x5, 0x04, 0x15, 0x92, 0xE5, 0x35, 0x95, 0x32, 0xE4, 0x44, 0x44, 0x63, 0x31
};

const uint8_t aid[] = 
{
    0xA0, 0x00, 0x00, 0x00, 0x04, 0x10, 0x10
};


void test_one()
{
    TLVS tlvs;
    TLVNode *parent = tlvs.addTLV(0x6f);
    tlvs.addTLV(parent, 0x84, df_name, sizeof(df_name));
    tlvs.addTLVCopy(parent, 0x84, df_name, sizeof(df_name));

    TLVNode *fci = tlvs.addTLV(parent, 0xA5);
    tlvs.addTLV(fci, 0x87);
    tlvs.addTLV(fci, 0x4f, aid, sizeof(aid));
    tlvs.addTLVCopy(fci, 0x4f, aid, sizeof(aid));
}



// Test strings
const char* tlvDataStr = "6f298407a0000000043060a51e50074d41455354524f5f2d046465656e9f38039f5c08bf0c059f4d020b0a";

const char* tlvData1Str = "77 16 82 02 19 80 94 10 08 01 03 03 10 01 02 00 10 04 04 00 20 01 01 01 90 00";


uint8_t tlvData[] = 
{
    0x6F,0x48,0x84,0x07,0xA0,0x00,0x00,0x00,0x04,
    0x10,0x10,0xA5,0x3D,0x50,0x0A,0x4D,0x41,0x53,
    0x54,0x45,0x52,0x43,0x41,0x52,0x44,0x87,0x01,
    0x01,0x5F,0x2D,0x02,0x65,0x6E,0x9F,0x11,0x01,
    0x01,0x9F,0x12,0x0A,0x4D,0x41,0x53,0x54,0x45,
    0x52,0x43,0x41,0x52,0x44,0xBF,0x0C,0x15,0x9F,
    0x5D,0x03,0x00,0x00,0x00,0x9F,0x4D,0x02,0x0B,
    0x0A,0x9F,0x6E,0x07,0x08,0x40,0x00,0x00,0x30,0x30,0x00
};

uint8_t encode_buffer[255];
uint8_t decode_buffer[255];
uint8_t long_buffer[1024];

// Indefinite length form
const uint8_t tlvIndefLen[] = 
{
    0x6F,0x80,0x84,0x07,0x00,0x00
};

// Too long
const uint8_t tlvLongLen[] = 
{
    0x6F,0x83,0x84,0x07,0x05,0x55
};


// 3 Tag bytes
const uint8_t tlvLongTag[] = 
{
    0x7F,0x83,0x84,0x07,0x05,0x55
};


void test_two()
{
    // Test bad tags and length
    TLVS tlvs;

    // Primitive with a child
    TLVNode *parent = tlvs.addTLV(0x6f);
    TLVNode *child = tlvs.addTLV(parent, 0x1e, df_name, sizeof(df_name));
    child = tlvs.addTLV(child, 0x4f, aid, sizeof(aid));
    tlvs.encodeTLVs(encode_buffer, sizeof(encode_buffer));
    assert(tlvs.errorValue());

    // "3 byte" tag
    printf("3 byte tag\n");
    tlvs.reset();
    tlvs.addTLV(0x7f82);
    tlvs.encodeTLVs(encode_buffer, sizeof(encode_buffer));
    assert(tlvs.errorValue());

    // Indefinite form for length
    printf("Indef len form\n");
    tlvs.reset();
    assert(!tlvs.errorValue());
    tlvs.decodeTLVs(tlvIndefLen, sizeof(tlvIndefLen));
    assert(tlvs.errorValue());


    // Length value that is too long ?> 2 bytes
    printf("Decode long length\n");
    tlvs.reset();
    tlvs.decodeTLVs(tlvLongLen, sizeof(tlvLongLen));
    assert(tlvs.errorValue());

    // 3 byte tag 
    printf("Decode 3 byte tag\n");
    tlvs.reset();
    tlvs.decodeTLVs(tlvLongTag, sizeof(tlvLongTag));
    assert(tlvs.errorValue());

}


void test_three()
{
    TLVS tlvs;
    TLVNode *parent = tlvs.addTLV(0x6f);

    TLVNode *child = tlvs.addTLV(parent, 0x84, df_name, sizeof(df_name));

    TLVNode *fci = tlvs.addTLV(parent, 0xA5);
    child = tlvs.addTLV(fci, 0x87);
    child = tlvs.addTLV(fci, 0x4f, aid, sizeof(aid));
}


void test_four()
{
    TLVS tlvs;

    TLVNode *parent = tlvs.addTLV(0x6f);
    size_t len = tlvs.encodeTLVs(encode_buffer, sizeof(encode_buffer));
    assert(!tlvs.errorValue());
    printf("data len = %d", len);
    TLVS::printHex(encode_buffer, len);
    printf("\n");
}    

void test_five()
{
    int data_size = TLVS::hexToBin(tlvDataStr, decode_buffer, sizeof(decode_buffer));
    TLVS::printHex(decode_buffer, data_size);
    printf("\n");

    TLVS tlvs;
    tlvs.decodeTLVs(decode_buffer, data_size);
    assert(!tlvs.errorValue());
    TLVS::printTLV(tlvs.firstTLV());
}
    
void test_six()
{
    int data_size = TLVS::hexToBin(tlvDataStr, decode_buffer, sizeof(decode_buffer));
    printf("Original encoding\n");
    TLVS::printHex(decode_buffer, data_size);
    printf("\n");

    TLVS tlvs;
    tlvs.decodeTLVs(decode_buffer, data_size);
    assert(!tlvs.errorValue());
    TLVS::printTLV(tlvs.firstTLV());

    data_size = tlvs.encodeTLVs(encode_buffer, sizeof(encode_buffer));
    assert(!tlvs.errorValue());
    printf("Transformed encoding\n");
    TLVS::printHex(encode_buffer, data_size);
    printf("\n");
}

void test_seven()
{
    TLVS tlvs;
    tlvs.decodeTLVs(tlvData, sizeof(tlvData));
    assert(!tlvs.errorValue());
    printf("TLVData:\n");
    TLVS::printTLV(tlvs.firstTLV());
    printf("\n");
}

uint8_t v1[] = { 0x1 };
uint8_t v2[] = { 0x2 };
uint8_t v3[] = { 0x3 };
uint8_t v4[] = { 0x4 };

void test_find()
{
    TLVS tlvs;
    TLVNode *tlvNodeTop, *tlvNode;

    // Find empty
    tlvNode = tlvs.findTLV(0x80);
    assert(tlvNode == NULL); 

    // Find and find next
    tlvNodeTop = tlvs.addTLV(0xBf01);

    tlvNode = tlvs.addTLV(tlvNodeTop, 0x8A);
    tlvs.addTLV(tlvNode, 0x8A, v2, sizeof(v2));
    tlvNode = tlvs.addTLV(tlvNode, 0x8B);
    tlvs.addTLV(tlvNode, 0x8B, v2, sizeof(v2));

    tlvNode = tlvs.addTLV(tlvNodeTop, 0x8B);
    tlvs.addTLV(tlvNode, 0x8A, v3, sizeof(v3));
    tlvs.addTLV(tlvNode, 0x8B, v4, sizeof(v4));

    tlvNode = tlvs.addTLV(tlvNode, 0x10);
    tlvNode = tlvs.addTLV(tlvNode, 0x11);
    tlvs.addTLV(tlvNode, 0x12);
    tlvs.addTLV(tlvNode, 0x12);
    tlvs.addTLV(0x12);

    tlvNode = tlvs.findTLV(0x80);
    assert(tlvNode == NULL); 

    tlvNode = tlvs.findTLV(0x8A);
    assert(tlvNode != NULL); 
    assert(tlvNode->getTag() == 0x8A);
    assert(tlvNode->getValue() == NULL);

    tlvNode = tlvs.findNextTLV(tlvNode);
    assert(tlvNode != NULL); 
    assert(tlvNode->getTag() == 0x8A);
    assert(tlvNode->getValue() != NULL);
    assert(tlvNode->getValue()[0] == 2);

    tlvNode = tlvs.findNextTLV(tlvNode);
    assert(tlvNode != NULL); 
    assert(tlvNode->getTag() == 0x8A);
    assert(tlvNode->getValue() != NULL);
    assert(tlvNode->getValue()[0] == 3);

    tlvNode = tlvs.findNextTLV(tlvNode);
    assert(tlvNode == NULL); 

    tlvNode = tlvs.findTLV(0x8B);
    assert(tlvNode != NULL); 
    assert(tlvNode->getTag() == 0x8B);
    assert(tlvNode->getValue() == NULL);

    tlvNode = tlvs.findNextTLV(tlvNode);
    assert(tlvNode != NULL); 
    assert(tlvNode->getTag() == 0x8B);
    assert(tlvNode->getValue() != NULL);
    assert(tlvNode->getValue()[0] == 2);

    tlvNode = tlvs.findNextTLV(tlvNode);
    assert(tlvNode != NULL); 
    assert(tlvNode->getTag() == 0x8B);
    assert(tlvNode->getValue() == NULL);

    tlvNode = tlvs.findNextTLV(tlvNode);
    assert(tlvNode != NULL); 
    assert(tlvNode->getTag() == 0x8B);
    assert(tlvNode->getValue() != NULL);
    assert(tlvNode->getValue()[0] == 4);

    tlvNode = tlvs.findNextTLV(tlvNode);
    assert(tlvNode == NULL); 

    tlvNode = tlvs.findTLV(0x11);
    assert(tlvNode != NULL); 
    tlvNode = tlvs.findNextTLV(tlvNode);
    assert(tlvNode == NULL); 
}

void test_decode_cases()
{
    TLVS tlvs;
    TLVNode *tlvNode;
    size_t size;

    size = TLVS::hexToBin("100100", decode_buffer, sizeof(decode_buffer));
    tlvs.decodeTLVs(decode_buffer, size);
    assert(!tlvs.errorValue());
    tlvNode = tlvs.firstTLV();
    assert(tlvNode);
    assert(tlvNode->getTag() == 0x10);
    assert(tlvNode->getValueLength() == 1);
    assert(tlvNode->getValue()[0] == 0);

    tlvs.reset();
    size = TLVS::hexToBin("9F01021234", decode_buffer, sizeof(decode_buffer));
    tlvs.decodeTLVs(decode_buffer, size);
    assert(!tlvs.errorValue());
    tlvNode = tlvs.firstTLV();
    assert(tlvNode);
    assert(tlvNode->getTag() == 0x9F01);
    assert(tlvNode->getValueLength() == 2);
    assert(tlvNode->getValue()[0] == 0x12);
    assert(tlvNode->getValue()[1] == 0x34);

    // No data
    tlvs.reset();
    size = TLVS::hexToBin("1000", decode_buffer, sizeof(decode_buffer));
    tlvs.decodeTLVs(decode_buffer, size);
    assert(!tlvs.errorValue());
    tlvNode = tlvs.firstTLV();
    assert(tlvNode);
    assert(tlvNode->getTag() == 0x10);
    assert(tlvNode->getValueLength() == 0);

    // 2 byte length
    tlvs.reset();
    size = TLVS::hexToBin("12820101", long_buffer, sizeof(long_buffer));
    memset(long_buffer + size, 0, 257);
    size += 257;
    tlvs.decodeTLVs(long_buffer, size);
    assert(!tlvs.errorValue());
    tlvNode = tlvs.firstTLV();
    TLVS::printTLV(tlvNode);
    assert(tlvNode);
    assert(tlvNode->getTag() == 0x12);
    printf("val len = %d\n", tlvNode->getValueLength());
    assert(tlvNode->getValueLength() == 257);
    bool equal = !memcmp(tlvNode->getValue(), long_buffer + size, 257);
    assert(equal);

    // leading zeros
    tlvs.reset();
    size = TLVS::hexToBin("00009F1001318A03414243", decode_buffer, sizeof(decode_buffer));
    tlvs.decodeTLVs(decode_buffer, size);
    assert(!tlvs.errorValue());
    tlvNode = tlvs.firstTLV();
    assert(tlvNode->getTag() == 0x9f10);
    assert(tlvNode->getValueLength() == 1);
    assert(tlvNode->getValue()[0] == 0x31);
    tlvNode = tlvs.nextTLV(tlvNode);
    assert(tlvNode->getTag() == 0x8a);
    assert(tlvNode->getValueLength() == 3);
    assert(tlvNode->getValue()[0] == 'A');
    assert(tlvNode->getValue()[1] == 'B');
    assert(tlvNode->getValue()[2] == 'C');

    // inter-element padding
    tlvs.reset();
    size = TLVS::hexToBin("9F10013100008A03414243", decode_buffer, sizeof(decode_buffer));
    tlvs.decodeTLVs(decode_buffer, size);
    assert(!tlvs.errorValue());
    tlvNode = tlvs.firstTLV();
    assert(tlvNode->getTag() == 0x9f10);
    assert(tlvNode->getValueLength() == 1);
    assert(tlvNode->getValue()[0] == 0x31);
    tlvNode = tlvs.nextTLV(tlvNode);
    assert(tlvNode->getTag() == 0x8a);
    assert(tlvNode->getValueLength() == 3);
    assert(tlvNode->getValue()[0] == 'A');
    assert(tlvNode->getValue()[1] == 'B');
    assert(tlvNode->getValue()[2] == 'C');

    // TODO: test trailing zeros
    tlvs.reset();
    size = TLVS::hexToBin("9F1001318A034142430000000", decode_buffer, sizeof(decode_buffer));
    tlvs.decodeTLVs(decode_buffer, size);
    assert(!tlvs.errorValue());
    tlvNode = tlvs.firstTLV();
    assert(tlvNode->getTag() == 0x9f10);
    assert(tlvNode->getValueLength() == 1);
    assert(tlvNode->getValue()[0] == 0x31);
    tlvNode = tlvs.nextTLV(tlvNode);
    assert(tlvNode->getTag() == 0x8a);
    assert(tlvNode->getValueLength() == 3);
    assert(tlvNode->getValue()[0] == 'A');
    assert(tlvNode->getValue()[1] == 'B');
    assert(tlvNode->getValue()[2] == 'C');
}

void test_encode_cases()
{
    TLVS tlvs;
    TLVNode *tlvNode, *tlvNode1;
    size_t data_size, size;

    // Empty
    data_size = tlvs.encodeTLVs(encode_buffer, sizeof(encode_buffer));
    assert(data_size == 0);
    assert(!tlvs.errorValue());

    // Nested
    tlvs.reset();
    tlvNode = tlvs.addTLV(0xBf10);
    tlvs.addTLV(tlvNode, 0x8a, (uint8_t*)"ABC", 3);
    data_size = tlvs.encodeTLVs(encode_buffer, sizeof(encode_buffer));
    size = TLVS::hexToBin("BF10058A03414243", decode_buffer, sizeof(decode_buffer));
    assert(size == data_size);
    assert(!memcmp(encode_buffer, decode_buffer, size));

    // Nested with empty
    tlvs.reset();
    tlvNode = tlvs.addTLV(0xBf10);
    tlvs.addTLV(tlvNode, 0x8A, (uint8_t*)"ABC", 3);
    tlvs.addTLV(tlvNode, 0x8B);
    data_size = tlvs.encodeTLVs(encode_buffer, sizeof(encode_buffer));
    size = TLVS::hexToBin("BF10078A034142438B00", decode_buffer, sizeof(decode_buffer));
    assert(size == data_size);
    assert(!memcmp(encode_buffer, decode_buffer, size));

    uint8_t one[] = { 0x01 };
    uint8_t two[] = { 0x02 };
    uint8_t one_two[] = { 0x01, 0x02 };
    uint8_t ff[] = { 0xff };


    // Larger, duplicate tags
    tlvs.reset();
    tlvNode = tlvs.addTLV(0xBf01);
    tlvs.addTLV(tlvNode, 0x8A, one, 1);
    tlvs.addTLV(tlvNode, 0x8B, (uint8_t*)"ABC", 3);
    tlvs.addTLV(tlvNode, 0x8A, two, 1);
    tlvs.addTLV(tlvNode, 0x8B, (uint8_t*)"DEF", 3);
    tlvNode1 = tlvs.addTLV(tlvNode, 0x10);
    tlvs.addTLV(tlvNode1, 0x11, one_two, 2);
    tlvs.addTLV(0x11, ff, 1);

    data_size = tlvs.encodeTLVs(encode_buffer, sizeof(encode_buffer));
    TLVS::printHex(encode_buffer, data_size);
    printf("\n");

    size = TLVS::hexToBin("Bf01168a01018b034142438a01028b034445461004110201021101ff", 
                           decode_buffer, sizeof(decode_buffer));
    assert(size == data_size);
    TLVS::printHex(decode_buffer, data_size);
    printf("\n");
    assert(!memcmp(encode_buffer, decode_buffer, size));
}


int main() {
    printf("Test databuffer\n");
    test_databuffer();
    printf("\n");

    printf("Test tags\n");
    test_tags();
    printf("\n");

    printf("Test 1\n");
    test_one();
    printf("\n");

    printf("Test 2\n");
    test_two();
    printf("\n");

    printf("Test 3\n");
    test_three();
    printf("\n");

    printf("Test 4\n");
    test_four();
    printf("\n");

    printf("Test 5\n");
    test_five();
    printf("\n");

    printf("Test 6\n");
    test_six();
    printf("\n");

    printf("Test 7\n");
    test_seven();
    printf("\n");

    printf("Test find\n");
    test_find();

    printf("Test Decode Cases\n");
    test_decode_cases();
    printf("\n");

    printf("Test encode cases\n");
    test_encode_cases();
    printf("\n");
}
    
