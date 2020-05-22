#pragma once

#include <sys/types.h>

class SerialCodec
{
public:
    // return size of encoded data in buffer
    virtual size_t size() = 0;

    // reset internal state
    virtual void reset() = 0;
};

class SerialEncoder : public SerialCodec
{
public:
    // write and encode data to buffer
    virtual size_t encode(const void *src, size_t sz) = 0;

    // read encoded data from buffer
    virtual size_t read_encoded(void *dest, size_t sz) = 0;
};

class SerialDecoder : public SerialCodec
{
public:
    enum ErrorType {
        DataAccepted,
        DataDropped,

        ErrorOverflow,
        ErrorSize,
        ErrorCRC,
    };

    // decode encoded data and write packet to buffer
    virtual ErrorType decode(const void *src, size_t sz) = 0;

    // read decoded data from buffer
    virtual size_t read_decoded(void *dest, size_t sz) = 0;
};
