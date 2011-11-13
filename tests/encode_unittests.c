#include <stdio.h>
#include <string.h>
#include "pb_encode.h"
#include "unittests.h"
#include "unittestproto.pb.h"

bool streamcallback(pb_ostream_t *stream, const uint8_t *buf, size_t count)
{
    /* Allow only 'x' to be written */
    while (count--)
    {
        if (*buf++ != 'x')
            return false;
    }
    return true;
}

bool fieldcallback(pb_ostream_t *stream, const pb_field_t *field, const void *arg)
{
    int value = 0x55;
    if (!pb_encode_tag_for_field(stream, field))
        return false;
    return pb_encode_varint(stream, value);
}

bool crazyfieldcallback(pb_ostream_t *stream, const pb_field_t *field, const void *arg)
{
    /* This callback writes different amount of data the second time. */
    uint32_t *state = (uint32_t*)arg;
    *state <<= 8;
    if (!pb_encode_tag_for_field(stream, field))
        return false;
    return pb_encode_varint(stream, *state);
}

/* Check that expression x writes data y.
 * Y is a string, which may contain null bytes. Null terminator is ignored.
 */
#define WRITES(x, y) \
memset(buffer, 0xAA, sizeof(buffer)), \
s = pb_ostream_from_buffer(buffer, sizeof(buffer)), \
(x) && \
memcmp(buffer, y, sizeof(y) - 1) == 0 && \
buffer[sizeof(y) - 1] == 0xAA

int main()
{
    int status = 0;
    
    {
        uint8_t buffer1[] = "foobartest1234";
        uint8_t buffer2[sizeof(buffer1)];
        pb_ostream_t stream = pb_ostream_from_buffer(buffer2, sizeof(buffer1));
        
        COMMENT("Test pb_write and pb_ostream_t");
        TEST(pb_write(&stream, buffer1, sizeof(buffer1)));
        TEST(memcmp(buffer1, buffer2, sizeof(buffer1)) == 0);
        TEST(!pb_write(&stream, buffer1, 1));
        TEST(stream.bytes_written == sizeof(buffer1));
    }
    
    {
        uint8_t buffer1[] = "xxxxxxx";
        pb_ostream_t stream = {&streamcallback, 0, SIZE_MAX, 0};
        
        COMMENT("Test pb_write with custom callback");
        TEST(pb_write(&stream, buffer1, 5));
        buffer1[0] = 'a';
        TEST(!pb_write(&stream, buffer1, 5));
    }
    
    {
        uint8_t buffer[30];
        pb_ostream_t s;
        
        COMMENT("Test pb_encode_varint")
        TEST(WRITES(pb_encode_varint(&s, 0), "\0"));
        TEST(WRITES(pb_encode_varint(&s, 1), "\1"));
        TEST(WRITES(pb_encode_varint(&s, 0x7F), "\x7F"));
        TEST(WRITES(pb_encode_varint(&s, 0x80), "\x80\x01"));
        TEST(WRITES(pb_encode_varint(&s, UINT32_MAX), "\xFF\xFF\xFF\xFF\x0F"));
        TEST(WRITES(pb_encode_varint(&s, UINT64_MAX), "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01"));
    }
    
    {
        uint8_t buffer[30];
        pb_ostream_t s;
        
        COMMENT("Test pb_encode_tag")
        TEST(WRITES(pb_encode_tag(&s, PB_WT_STRING, 5), "\x2A"));
        TEST(WRITES(pb_encode_tag(&s, PB_WT_VARINT, 99), "\x98\x06"));
    }
    
    {
        uint8_t buffer[30];
        pb_ostream_t s;
        pb_field_t field = {10, PB_LTYPE_SVARINT};
        
        COMMENT("Test pb_encode_tag_for_field")
        TEST(WRITES(pb_encode_tag_for_field(&s, &field), "\x50"));
        
        field.type = PB_LTYPE_FIXED64;
        TEST(WRITES(pb_encode_tag_for_field(&s, &field), "\x51"));
        
        field.type = PB_LTYPE_STRING;
        TEST(WRITES(pb_encode_tag_for_field(&s, &field), "\x52"));
        
        field.type = PB_LTYPE_FIXED32;
        TEST(WRITES(pb_encode_tag_for_field(&s, &field), "\x55"));
    }
    
    {
        uint8_t buffer[30];
        pb_ostream_t s;
        
        COMMENT("Test pb_encode_string")
        TEST(WRITES(pb_encode_string(&s, (const uint8_t*)"abcd", 4), "\x04""abcd"));
        TEST(WRITES(pb_encode_string(&s, (const uint8_t*)"abcd\x00", 5), "\x05""abcd\x00"));
        TEST(WRITES(pb_encode_string(&s, (const uint8_t*)"", 0), "\x00"));
    }
    
    {
        uint8_t buffer[30];
        pb_ostream_t s;
        uint8_t value = 1;
        int8_t svalue = -1;
        int32_t max = INT32_MAX;
        int32_t min = INT32_MIN;
        int64_t lmax = INT64_MAX;
        int64_t lmin = INT64_MIN;
        pb_field_t field = {1, PB_LTYPE_VARINT, 0, 0, sizeof(value)};
        
        COMMENT("Test pb_enc_varint and pb_enc_svarint")
        TEST(WRITES(pb_enc_varint(&s, &field, &value), "\x01"));
        TEST(WRITES(pb_enc_svarint(&s, &field, &svalue), "\x01"));
        TEST(WRITES(pb_enc_svarint(&s, &field, &value), "\x02"));
        
        field.data_size = sizeof(max);
        TEST(WRITES(pb_enc_svarint(&s, &field, &max), "\xfe\xff\xff\xff\x0f"));
        TEST(WRITES(pb_enc_svarint(&s, &field, &min), "\xff\xff\xff\xff\x0f"));
        
        field.data_size = sizeof(lmax);
        TEST(WRITES(pb_enc_svarint(&s, &field, &lmax), "\xFE\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01"));
        TEST(WRITES(pb_enc_svarint(&s, &field, &lmin), "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01"));
    }
    
    {
        uint8_t buffer[30];
        pb_ostream_t s;
        float fvalue;
        double dvalue;
        
        COMMENT("Test pb_enc_fixed32 using float")
        fvalue = 0.0f;
        TEST(WRITES(pb_enc_fixed32(&s, NULL, &fvalue), "\x00\x00\x00\x00"))
        fvalue = 99.0f;
        TEST(WRITES(pb_enc_fixed32(&s, NULL, &fvalue), "\x00\x00\xc6\x42"))
        fvalue = -12345678.0f;
        TEST(WRITES(pb_enc_fixed32(&s, NULL, &fvalue), "\x4e\x61\x3c\xcb"))
    
        COMMENT("Test pb_enc_fixed64 using double")
        dvalue = 0.0;
        TEST(WRITES(pb_enc_fixed64(&s, NULL, &dvalue), "\x00\x00\x00\x00\x00\x00\x00\x00"))
        dvalue = 99.0;
        TEST(WRITES(pb_enc_fixed64(&s, NULL, &dvalue), "\x00\x00\x00\x00\x00\xc0\x58\x40"))
        dvalue = -12345678.0;
        TEST(WRITES(pb_enc_fixed64(&s, NULL, &dvalue), "\x00\x00\x00\xc0\x29\x8c\x67\xc1"))
    }
    
    {
        uint8_t buffer[30];
        pb_ostream_t s;
        struct { size_t size; uint8_t bytes[5]; } value = {5, {'x', 'y', 'z', 'z', 'y'}};
    
        COMMENT("Test pb_enc_bytes")
        TEST(WRITES(pb_enc_bytes(&s, NULL, &value), "\x05xyzzy"))
        value.size = 0;
        TEST(WRITES(pb_enc_bytes(&s, NULL, &value), "\x00"))
    }
    
    {
        uint8_t buffer[30];
        pb_ostream_t s;
        char value[] = "xyzzy";
        
        COMMENT("Test pb_enc_string")
        TEST(WRITES(pb_enc_string(&s, NULL, &value), "\x05xyzzy"))
        value[0] = '\0';
        TEST(WRITES(pb_enc_string(&s, NULL, &value), "\x00"))
    }
    
    {
        uint8_t buffer[10];
        pb_ostream_t s;
        IntegerArray msg = {{0}, 5, {1, 2, 3, 4, 5}};
        
        COMMENT("Test pb_encode with int32 array")
        
        TEST(WRITES(pb_encode(&s, IntegerArray_msg, &msg), "\x0A\x05\x01\x02\x03\x04\x05"))
        
        msg.data_count = 0;
        TEST(WRITES(pb_encode(&s, IntegerArray_msg, &msg), ""))
        
        msg.data_count = 10;
        TEST(!pb_encode(&s, IntegerArray_msg, &msg))
    }
    
    {
        uint8_t buffer[10];
        pb_ostream_t s;
        FloatArray msg = {{0}, 1, {99.0f}};
        
        COMMENT("Test pb_encode with float array")
        
        TEST(WRITES(pb_encode(&s, FloatArray_msg, &msg),
                    "\x0A\x04\x00\x00\xc6\x42"))
        
        msg.data_count = 0;
        TEST(WRITES(pb_encode(&s, FloatArray_msg, &msg), ""))
        
        msg.data_count = 3;
        TEST(!pb_encode(&s, FloatArray_msg, &msg))
    }
    
    {
        uint8_t buffer[10];
        pb_ostream_t s;
        CallbackArray msg;
        
        msg.data.funcs.encode = &fieldcallback;
        
        COMMENT("Test pb_encode with callback field.")
        TEST(WRITES(pb_encode(&s, CallbackArray_msg, &msg), "\x08\x55"))
    }
    
    {
        uint8_t buffer[10];
        pb_ostream_t s;
        IntegerContainer msg = {{0}, {{0}, 5, {1,2,3,4,5}}};
        
        COMMENT("Test pb_encode with packed array in a submessage.")
        TEST(WRITES(pb_encode(&s, IntegerContainer_msg, &msg),
                    "\x0A\x07\x0A\x05\x01\x02\x03\x04\x05"))
    }
    
    {
        uint8_t buffer[10];
        pb_ostream_t s;
        CallbackContainer msg;
        CallbackContainerContainer msg2;
        uint32_t state = 1;
        
        msg.submsg.data.funcs.encode = &fieldcallback;
        msg2.submsg.submsg.data.funcs.encode = &fieldcallback;
        
        COMMENT("Test pb_encode with callback field in a submessage.")
        TEST(WRITES(pb_encode(&s, CallbackContainer_msg, &msg), "\x0A\x02\x08\x55"))
        TEST(WRITES(pb_encode(&s, CallbackContainerContainer_msg, &msg2),
                    "\x0A\x04\x0A\x02\x08\x55"))
        
        /* Misbehaving callback: varying output between calls */
        msg.submsg.data.funcs.encode = &crazyfieldcallback;
        msg.submsg.data.arg = &state;
        msg2.submsg.submsg.data.funcs.encode = &crazyfieldcallback;
        msg2.submsg.submsg.data.arg = &state;
        
        TEST(!pb_encode(&s, CallbackContainer_msg, &msg))
        state = 1;
        TEST(!pb_encode(&s, CallbackContainerContainer_msg, &msg2))
    }
    
    if (status != 0)
        fprintf(stdout, "\n\nSome tests FAILED!\n");
    
    return status;
}
