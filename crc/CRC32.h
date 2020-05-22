/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#pragma once

#include <cinttypes>
#include <do_not_copy.h>
#include <sys/types.h>

class CRC32 : private do_not_copy
{
public:
    explicit CRC32();
    explicit CRC32(const void *data, size_t sz);

    void reset();
    uint32_t result() const;

    void add(const void *data, size_t sz);

private:
    uint32_t _value;
};
