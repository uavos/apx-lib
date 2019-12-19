#pragma once

#include "Xdb.h"

#include <containers/OrderedList.hpp>

namespace xdb {

class XdbSub : public OrderedListNode<XdbSub *>
{
public:
    explicit XdbSub(const XdbMetaData *meta);
    virtual ~XdbSub();

    bool match(uid_t uid) const;
    uid_t uid() const { return _meta->uid; }
    int id() const override { return uid(); }

    virtual bool update(const void *data, uint16_t size);

private:
    const XdbMetaData *_meta{nullptr};

    uint8_t *_data{nullptr};
    void copy(const void *data, uint16_t size);
};

} // namespace xdb
