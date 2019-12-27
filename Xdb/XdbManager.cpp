#include "XdbManager.h"

using namespace xdb;

List<XdbPub *> XdbManager::_pubs;
OrderedList<XdbSub *> XdbManager::_subs;

void XdbManager::subscribe(XdbSub *n)
{
    _subs.add_ordered(n);
}

void XdbManager::unsubscribe(XdbSub *n)
{
    _subs.remove(n);
}

void XdbManager::advertise(XdbPub *n)
{
    _pubs.add(n);
}

void XdbManager::unadvertise(XdbPub *n)
{
    _pubs.remove(n);
}

void XdbManager::publish(uid_t uid, const void *data, uint16_t size)
{
    bool accepted = false;
    for (auto i : _subs) {
        if (i->match(uid)) {
            accepted = i->update(data, size);
            continue;
        }
        if (accepted)
            break;
    }
}
