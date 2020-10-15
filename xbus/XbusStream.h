#pragma once

#include <do_not_copy.h>
#include <sys/types.h>

class XbusStream : public do_not_copy
{
public:
    explicit XbusStream(size_t size)
        : _size(size)
    {}

    inline void reset(size_t pos = 0) { _pos = pos > _size ? _size : pos; }
    inline size_t size() const { return _size; }
    inline size_t pos() const { return _pos; }
    inline size_t available() const { return _size - _pos; }

protected:
    size_t _size;
    size_t _pos{0};
};
