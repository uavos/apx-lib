#include "XdbSub.h"
#include "XdbManager.h"

#include <cstring>

using namespace xdb;

XdbSub::XdbSub(const XdbMetaData *meta)
    : _meta(meta)
{
    XdbManager::subscribe(this);
}

XdbSub::~XdbSub()
{
    XdbManager::unsubscribe(this);
}

bool XdbSub::match(uid_t uid) const
{
    return _meta->uid == uid;
}

bool XdbSub::update(const void *data, uint16_t size)
{
    copy(data, size);
    return true;
}

void XdbSub::copy(const void *data, uint16_t size)
{
    if (size > _meta->size)
        return;
    if (nullptr == _data) {
        _data = new uint8_t[_meta->size];
        if (nullptr == _data) {
            return;
        }
    }
    memcpy(_data, data, size);
}
