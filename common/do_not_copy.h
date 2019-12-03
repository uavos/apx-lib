/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#pragma once

class do_not_copy
{
public:
    do_not_copy() {}

private:
    // Disallow copy construction and move assignment
    do_not_copy(const do_not_copy &) = delete;
    do_not_copy &operator=(const do_not_copy &) = delete;
    do_not_copy(do_not_copy &&) = delete;
    do_not_copy &operator=(do_not_copy &&) = delete;
};
