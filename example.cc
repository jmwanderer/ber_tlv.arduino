//
// example.cc - Encode and decode a set of TLVs
//
// Copyright (c) 2025 James Wanderer
// 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "tlv.h"

// Buffer to encode into, decode from.
uint8_t buffer[255];

int main()
{
    TLVS tlvs;
    TLVNode *tlvNode, *childNode;
    size_t data_size;

    // Data for our TLVs
    uint8_t abc[] = { 0x65, 0x66, 0x67 };
    uint8_t def[] = { 0x68, 0x69, 0x6a };

    // Build a set of TLVs
    tlvNode = tlvs.addTLV(0xbf10);  // Tag 0xbf10 with 2 child TLVs
    tlvs.addTLV(tlvNode, 0x8a, abc, sizeof(abc));   // Tag 0x8a
    tlvs.addTLV(tlvNode, 0x8b, def, sizeof(def));   // Tag 0x8b

    // Encode TLVs into the buffer
    data_size = tlvs.encodeTLVs(buffer, sizeof(buffer));

    // Decode TLVs (automaticaly resets the TLVS)
    tlvs.decodeTLVs(buffer, sizeof(buffer));

    // Dump the decoded TLV structure
    tlvNode = tlvs.firstTLV();
    printf("TLV Node %x\n", tlvNode->getTag());
    for (childNode = tlvNode->firstChild(); childNode; childNode = tlvNode->nextChild(childNode)) {
        printf("Child Node %x\n", childNode->getTag());
    }
    TLVS::printTLV(tlvNode);
}

