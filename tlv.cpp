//
// tlv.cc - Simple BER TLV encoder / decoder
//
// Copyright (c) 2025 James Wanderer
// 
// See tlv.h for basic information.
//
#include <Arduino.h>

#include "tlv.h"

//
// TLVS: Represents a list of TLVs
//
// Supports:
//  - adding TLVs at the top level or to existing TLVs
//  - encode TLVs to a buffer
//  - decode TLVs from a buffer
//  - access TLV hierarchy
//

TLVS::TLVS()
{
    error_value = 0;
    // Dummy is always constructed. 
    // Set to avoid error flag when encoding.
    dummy_node.tag = TLV_TYPE_MASK;
}

//
// Reset and free contents for TLVS re-use
void TLVS::reset()
{
    error_value = 0;
    dummy_node.freeContents();
}

int TLVS::errorValue()
{
    return error_value;
}

TLVNode* TLVS::addTLV(uint16_t tag)
{
    return  addTLV(NULL, tag);
}

TLVNode* TLVS::addTLV(TLVNode* parent, uint16_t tag)
{
    TLVNode *node = new TLVNode(tag, 0);
    if (parent == NULL) {
        // Add top level TLV
        dummy_node.addChild(node);
    } else {
        // Check that parent tag is a constructed form, report error
        if (!Tag::tagConstructed(parent->tag)) {
            markError(ERROR_PRIMIVE_TYPE);
        }
        parent->addChild(node);
    }
    return node;
}

TLVNode* TLVS::addTLV(uint16_t tag, const uint8_t *value, uint16_t value_length)
{
    return addTLV(NULL, tag, value, value_length);
}

TLVNode* TLVS::addTLVCopy(uint16_t tag, const uint8_t *value, uint16_t value_length)
{
    return addTLVCopy(NULL, tag, value, value_length);
}

TLVNode* TLVS::addTLV(TLVNode *parent, uint16_t tag, const uint8_t *value, uint16_t value_length)
{
    TLVNode* node = new TLVNode(tag, value_length);
    node->value = value;
    node->value_allocated = false;

    if (parent == NULL) {
        dummy_node.addChild(node);
    } else {
        if (!Tag::tagConstructed(parent->tag)) {
            markError(ERROR_PRIMIVE_TYPE);
        }
        parent->addChild(node);
    }
    return node;
}


//
// Allocate memory and copy the value
TLVNode* TLVS::addTLVCopy(TLVNode *parent, uint16_t tag, const uint8_t *value, uint16_t value_length)
{
    uint8_t *value_copy = NULL;
    if (value_length != 0) {
        value_copy = (uint8_t*) malloc(value_length);
        if (value_copy != NULL) {
            memcpy(value_copy, value, value_length);
        }
    }
    
    TLVNode* node = this->addTLV(parent, tag, value_copy, value_length);

    if (node->value != NULL) {
        // Important to note this after the call to addTLV
        node->value_allocated = true;
    }

    return node;
}


//
// Decode TLVs from the buffer
void TLVS::decodeTLVs(const uint8_t *buffer, size_t buffer_size)
{
    ReadBuffer dataBuffer(buffer, buffer_size);
    reset();
    dummy_node.decodeTLVNode(this, dataBuffer);
}

//
// Encode TLVs to the buffer
size_t TLVS::encodeTLVs(uint8_t *buffer, size_t buffer_size)
{
    WriteBuffer dataBuffer(buffer, buffer_size);
    TLVNode *node = dummy_node.child;
    for (node = dummy_node.child; node != NULL; node = node->next) {
        node->encodeTLVNode(this, dataBuffer);
    }
    return dataBuffer.pos;
}


TLVNode* TLVS::firstTLV()
{
    return dummy_node.firstChild();
}

TLVNode* TLVS::nextTLV(TLVNode* child)
{
    return dummy_node.nextChild(child);
}

TLVNode* TLVS::findTLV(uint16_t tag)
{
    return findTLVHelper(&dummy_node, tag);
}

TLVNode* TLVS::findNextTLV(TLVNode* node)
{
    if (node == NULL) {
        return NULL;
    }
    return findTLVHelper(node, node->tag);
}

TLVNode* TLVS::findTLVHelper(TLVNode* node, uint16_t tag)
{
    while (node != NULL) {

        // Move to next node
        if (node->child != NULL) {
            // Move to child
            node = node->child;
        } else if (node->next != NULL) {
            // Check next sibling
            node = node->next;
        } else {
            // Next sibling of a parent
            do {
                node = node->parent;
            } while (node != NULL && node->next == NULL);
            
            if (node != NULL) {
                node = node->next;
            }
        }
        
        if (node != NULL && node->getTag() == tag) {
            return node;
        }
    }
    return NULL;
}

void TLVS::printHex(const uint8_t* data, size_t length)
{
    if (data == NULL)
        return;

    for (int i = 0; i < length; i++) {
        if (i != 0) {
            Serial.print(" ");
        }
        Serial.print(data[i] >> 4, HEX);
        Serial.print(data[i] & 0xf, HEX);
    }
}

void TLVS::printValue(const uint8_t* data, size_t length)
{
    if (data == NULL)
        return;

    bool ascii = true;
    for (int i = 0; i < length; i++) {
        if (data[i] < 32 || data[i] > 126) {
            ascii = false;
        }
    }

    if (ascii) {
        for (int i = 0; i < length; i++) {
            char buffer[3];
            buffer[0] = (char) data[i];
            buffer[1] = ' ';
            buffer[2] = '\0';
            Serial.print(buffer);
        }
    } else {
        printHex(data, length);
    }
}


void TLVS::printTLV(TLVNode* node, int indent)
{
    for (int i = 0; i < indent; i++)
        Serial.print("    ");

    if (node == NULL) {
        Serial.println("NULL pointer for TLVNode....");
        return;
    }

    Serial.print("Tag: ");
    Serial.print(node->getTag(), HEX);
    Serial.print(" Length: ");
    Serial.println(node->getValueLength(), HEX);
    TLVNode *child = node->firstChild();
    if (child == NULL) {
        for (int i = 0; i <= indent; i++)
            Serial.print("    ");
        printValue(node->getValue(), node->getValueLength());
        Serial.println("");
    }

    while (child != NULL) {
        printTLV(child, indent + 1);
        child = node->nextChild(child);
    }
}

size_t TLVS::hexToBin(const char* str, uint8_t* buffer, size_t buffer_size)
{
    char hex_str[3];
    size_t count = strlen(str);
    size_t str_index = 0;
    size_t buf_index = 0;

    while (str_index < count - 1) {
        if (str[str_index] == ' ') {
            str_index++;
            continue;
        }
            
        hex_str[0] = str[str_index++];
        hex_str[1] = str[str_index++];
        hex_str[2] = '\0';
        long val = strtol(hex_str, NULL, 16);
        if (buf_index < buffer_size)
            buffer[buf_index++] = val & 0xff;
    }
    return buf_index;
}

// 
// Save the first error in an encode or decode operation
void TLVS::markError(int error)
{
    // Save the first error
    if (error_value == 0) {
        error_value = error;
    }
}


//
// TLVNode: represents a TLV and possibly a value or nested TLVs
//

TLVNode::TLVNode(uint16_t tag, uint16_t length)
{
    this->tag = tag;
    this->value_length = length;
    value = NULL;
    next = NULL;
    child = NULL;
    parent = NULL;
    value_allocated = false;
}


TLVNode::~TLVNode()
{
    freeContents();
}

void TLVNode::freeContents()
{
    if (value_allocated) {
        free((uint8_t*)value);
        value = NULL;
        value_allocated = false;
    }

    // Cleanup kids
    while (child != NULL) {
        TLVNode *node = child;
        child = child->next;
        delete node;
    }
}

//
// Static function to parse a TLV tag from the buffer.
uint16_t TLVNode::parseTag(ReadBuffer &buffer, int *error)
{
    uint8_t byte =  0;
    uint16_t tag_value;

    *error = TLVS::ERROR_NONE;

    // Skip zeros
    while (byte == 0 && !buffer.atEnd()) {
        buffer.getByte(byte);
    }
    if (byte == 0) {
        // Ran out of data. Trailing 0s are OK.
        return 0;
    }
    tag_value = byte;

    if ((byte & TLV_TAG_MASK) != TLV_TAG_MASK)
        return tag_value;

    // Get a 2nd byte
    if (buffer.atEnd()) {
        *error = TLVS::ERROR_END_DATA;
    } else {
        buffer.getByte(byte);
        tag_value = (tag_value << 8) | byte;
    }

    // check encoding calls for no more tag bytes, report error
    if (byte & 0x80) {
        *error = TLVS::ERROR_TAG_LENGTH;
    }   

    return tag_value;
}

//
// Static function to encode a TLV tag to the buffer.
int TLVNode::encodeTag(uint16_t tag, WriteBuffer & buffer)
{
    int error = TLVS::ERROR_NONE;

    uint8_t byte = Tag::leading_byte(tag);
    if ((byte & TLV_TAG_MASK ) != TLV_TAG_MASK) {
        // Single byte tag
        buffer.putByte(byte);
    } else {
        // Two byte tag
        buffer.putByte(byte);
        buffer.putByte(tag & 0xff);

        // Check no more tag bytes, report error
        if (tag & 0x80) {
            error = TLVS::ERROR_TAG_LENGTH;
        }
    }
    return error;
}

//
// Static function to parse a length fromthe buffer
uint16_t TLVNode::parseLength(ReadBuffer &buffer, int *error)
{
    uint8_t byte;
    *error = TLVS::ERROR_NONE;

    buffer.getByte(byte);

    if ((byte & 0x80) == 0) {
        // Short definite form
        return TLV_LEN_MASK & byte;
    }

    // Check for indefinite and reserved form, report error
    if (byte == 0x80 || byte == 0xff) {
        *error = TLVS::ERROR_BAD_LENGTH;
    }

    // Long definte form
    uint16_t length = 0;
    uint8_t count = byte & 0x7F;

    // Check for long data, report error
    if (count > 2) {
        *error = TLVS::ERROR_LONG_DATA;
    }

    while (count-- > 0) {
        if (buffer.atEnd()) {
            *error = TLVS::ERROR_END_DATA;
        }
        buffer.getByte(byte);
        length = (length << 8) | (uint16_t) byte;
    }
    return length;
}

//
// Static function to encode a length to the buffer
int TLVNode::encodeLength(uint32_t length, WriteBuffer &buffer)
{
    if (length <= 127) {
        buffer.putByte(length & 0xff);
    } else {
        buffer.putByte(0x80 | 2);
        buffer.putByte((length >> 8) & 0xff);
        buffer.putByte(length & 0xff);
    }
    // check for long data, report error
    if (length > TLVS::MAX_DATA_LENGTH) {
        return TLVS::ERROR_LONG_DATA;
    }
    return TLVS::ERROR_NONE;
}


//
// Decode a child TLV
// Instance may be the dummy node if the "child" is a top level TLV.
//
void TLVNode::decodeTLVNode(TLVS *tlvs, ReadBuffer &buffer)
{
    int error = TLVS::ERROR_NONE;

    // Parse buffer and build a TLV tree.
    while (not buffer.atEnd()) {
        uint16_t tag = parseTag(buffer, &error);
        if (tag == 0) {
            // Buffer should be empty with trailing zeros.
            // Loop again to check.
            continue;
        }
        if (error) {
            tlvs->markError(error);
        }

        uint16_t len = parseLength(buffer, &error) ;
        if (error) {
            tlvs->markError(error);
        }

        // Ensure the reported length doesn't send us past the end of the buffer
        if (buffer.pos + len > buffer.buffer_size) {
            tlvs->markError(TLVS::ERROR_END_DATA);
            len = buffer.buffer_size - buffer.pos;
        }
        TLVNode *node = tlvs->addTLV(this, tag, buffer.position(), len);

        if (Tag::tagConstructed(tag)) {
            ReadBuffer valueBuffer(buffer, len);
            node->decodeTLVNode(tlvs, valueBuffer);
        }
        buffer.seek(len);
    }
}


void TLVNode::encodeTLVNode(TLVS *tlvs, WriteBuffer &buffer) {
    // Cases:
    // - has children: encode tag and length,  encode children
    // - has value: encode tag and length, write value
    // - zero length: encode tag and zero length

    // Encode Tag and length
    int error = encodeTag(tag, buffer);
    if (error) {
        tlvs->markError(error);
    }
    error = encodeLength(getValueLength(), buffer);
    if (error) {
        tlvs->markError(error);
    }

    if (firstChild() != NULL) {
        // Encode child TLVs
        TLVNode *child;
        for (child = firstChild(); child != NULL; child = nextChild(child)) {
            child->encodeTLVNode(tlvs, buffer);
        }
    } else {
        // Encode the value
        for (uint16_t i = 0; i < value_length; i++) {
            buffer.putByte(value[i]);
        }
    }
}

uint16_t TLVNode::getTag()
{
    return tag;
}

const uint8_t* TLVNode::getValue()
{
    if (child != NULL) {
        return NULL;
    }
    return value;
}
 
//
// Return the length of the data value
// May calculate and cache the size of the nested TLVs
uint32_t TLVNode::getValueLength()
{
    if (value_length == 0 && child != NULL) {
        // Calculate and cachle the size of child TLVs
        TLVNode *node;
        for (node = child; node != NULL; node = node->next) {
            value_length += node->getTotalBytes();
        }
    } 
    return value_length;
}

//
// Return the total size of the encoded TLV
uint32_t TLVNode::getTotalBytes()
{
    return Tag::numTagBytes(tag) + Tag::numLengthBytes(getValueLength()) + getValueLength();
}

 
//
// Returns NULL if no child TLVs
TLVNode* TLVNode::firstChild()
{
    return child;
}

//
// Returns NULL if no more child TLVs.
TLVNode* TLVNode::nextChild(TLVNode* child)
{
    return child->next;
}

TLVNode* TLVNode::findChild(uint16_t tag)
{
    TLVNode* node;
    for (node = firstChild(); node != NULL; node = nextChild(node)) {
        if (node->tag == tag) {
            return node;
        }
    }
    return NULL;
}

//
// Append a TLV to the end of the list
void TLVNode::addChild(TLVNode* node)
{
    if (child == NULL) {
        child = node;
        node->parent = this;
        return;
    }
    TLVNode *last_child = child;
    while (last_child->next != NULL) {
        last_child = last_child->next;
    }
    last_child->next = node;
    node->parent = this;

    // Number of child TLVs changed, ensure no length values are cached
    clearCachedSize();
}

// 
// Called when number of child TLVs have changed
void TLVNode::clearCachedSize()
{
    // Clear any calculated length values
    value_length = 0;
    if (parent != NULL) {
        parent->clearCachedSize();
    }
}


//
// Tag utility funcitons
//


//
// Return leading byte in a tag. 
// Byte 0 for 1 byte tags
// Byte 1 for 2 byte tags
uint8_t Tag::leading_byte(uint16_t tag)
{
    if ((tag & 0xff00) == 0) {
        return (uint8_t) tag;
    } else {
        return (uint8_t)(tag >> 8);
    }
}

bool Tag::tagConstructed(uint16_t tag)
{
    return leading_byte(tag) & TLV_TYPE_MASK;
}

bool Tag::tagClass(uint16_t tag)
{
    return leading_byte(tag) & TLV_TYPE_MASK;
}

// Return the number of bytes in a tag encoding
uint16_t Tag::numTagBytes(uint16_t tag)
{
    uint8_t byte = Tag::leading_byte(tag);
    if ((byte & TLV_TAG_MASK ) == TLV_TAG_MASK) {
        return 2;
    } else {
        return 1;
    }
}

// 
// Return the number of bytes in a length encoding
uint16_t Tag::numLengthBytes(uint16_t length)
{
    if (length > 127)
        return 3;
    return 1;
}

//
// ReadBuffer: utility for safely reading data from a buffer
//

ReadBuffer::ReadBuffer()
{
    this->buffer = NULL;
    this->buffer_size = 0;
    this->pos = 0;
}


ReadBuffer::ReadBuffer(const uint8_t *buffer, uint16_t size)
{
    this->buffer = buffer;
    this->buffer_size = size;
    this->pos = 0;
}

// Point to a portion of an existing buffer.
// Used to decode nexted TLVs.
ReadBuffer::ReadBuffer(ReadBuffer buffer, uint16_t size)
{
    this->buffer = buffer.buffer;
    this->pos = buffer.pos;
    this->buffer_size = buffer.pos + size;
    if (this->buffer_size > buffer.buffer_size)
        this->buffer_size = buffer.buffer_size;
}


// Return current read position.
const uint8_t * ReadBuffer::position()
{
    return buffer + pos;
}

// Change read position in buffer.
void ReadBuffer::seek(size_t count)
{
    pos += count;
    if (pos < 0) {
        pos = 0;
    } else if (pos > buffer_size) {
        pos = buffer_size;
    }
}

// Fetch a byte from the buffer.
// Return false if no more bytes to read
bool ReadBuffer::getByte(uint8_t &value)
{
    if (pos < buffer_size) {
        value = buffer[pos++];
        return true;
    }
    return false;
}

// Return true if no more space to write or data to read.
bool ReadBuffer::atEnd()
{
    return (pos >= buffer_size);
}


//
// WriteBuffer: utility for safely writing data to a buffer
//

WriteBuffer::WriteBuffer()
{
    this->buffer = NULL;
    this->buffer_size = 0;
    this->pos = 0;
}


WriteBuffer::WriteBuffer(uint8_t *buffer, uint16_t size)
{
    this->buffer = buffer;
    this->buffer_size = size;
    this->pos = 0;
}


// Return current write position.
uint8_t * WriteBuffer::position()
{
    return buffer + pos;
}

// Write a byte to the buffer
// Return false if out of space.
bool WriteBuffer::putByte(uint8_t value)
{
    if (pos < buffer_size) {
        buffer[pos++] = value;
        return true;
    }
    return false;
}

bool WriteBuffer::putBytes(const uint8_t *values, uint8_t len)
{
    bool ok = true;
    for (int i = 0; i < len; i++) {
        bool result = putByte(values[i]);
        ok = ok && result;
    }
    return ok;
}

