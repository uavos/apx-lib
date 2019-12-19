#pragma once

#include <sys/types.h>
#include <visibility.h>

namespace xdb {

typedef uint16_t uid_t;

struct XdbMetaData
{
    const uid_t uid;
    const uint16_t size;
};

} // namespace xdb

/**
 * Declare (prototype) the XDB metadata for a fact (used by code generators).
 *
 * @param _name		The name of the fact.
 */
#define XDB_DECLARE(_name) extern const struct xdb::XdbMetaData __xdb_##_name __EXPORT

/** 1116.29
 * Define (instantiate) the XDB metadata for a fact.
 *
 * The XDB metadata is used to help ensure that updates and
 * copies are accessing the right data.
 *
 * Note that there must be no more than one instance of this macro
 * for each topic.
 *
 * @param _uid      Unique ID of the fact
 * @param _size	    Struct size
 * @param _name		The name of the fact.
 * @param _struct	The structure the fcat provides.
 */
#define XDB_DEFINE(_uid, _struct, _name) \
    const struct xdb::XdbMetaData xdb::__xdb_##_name = { \
        _uid, \
        sizeof(_struct), \
    }; \
    struct hack

/**
 * Generates a pointer to the XDB metadata structure for
 * a given fact.
 *
 * The fact must have been declared previously in scope
 * with XDB_DECLARE().
 *
 * @param _name		The name of the fact.
 */
#define XDB_META(_name) xdb::__xdb_##_name
#define XDB_ID(_name) &XDB_META(_name)
