#include "XdbPub.h"
#include "XdbManager.h"

#include <cstring>

using namespace xdb;

XdbPub::XdbPub(const XdbMetaData *meta)
    : _meta(meta)
{
    XdbManager::advertise(this);
}

XdbPub::~XdbPub()
{
    XdbManager::unadvertise(this);
}

void XdbPub::publish(const void *data, uint16_t size)
{
    XdbManager::publish(_meta->uid, data, size);
}
