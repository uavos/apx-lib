#pragma once

#include "Xdb.h"

#include <containers/List.hpp>

namespace xdb {

class XdbPub : public ListNode<XdbPub *>
{
public:
    explicit XdbPub(const XdbMetaData *meta);
    virtual ~XdbPub();

    virtual void publish(const void *data, uint16_t size);

private:
    const XdbMetaData *_meta{nullptr};
};

} // namespace xdb
