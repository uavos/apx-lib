#include "XdbManager.h"
#include "XdbSub.h"

using namespace xdb;

XdbManager *XdbManager::_instance = nullptr;

XdbManager::XdbManager()
{
    _instance = this;
}

void XdbManager::initialize()
{
    if (_instance)
        return;
    _instance = new XdbManager();
}

void XdbManager::subscribe(XdbSub *n)
{
    _instance->_subs.add_ordered(n);
}

void XdbManager::unsubscribe(XdbSub *n)
{
    _instance->_subs.remove(n);
}

void XdbManager::advertise(XdbPub *n)
{
    _instance->_pubs.add(n);
}

void XdbManager::unadvertise(XdbPub *n)
{
    _instance->_pubs.remove(n);
}

void XdbManager::publish(uid_t uid, const void *data, uint16_t size)
{
    bool accepted = false;
    for (auto i : _instance->_subs) {
        if (i->match(uid)) {
            accepted = i->update(data, size);
            continue;
        }
        if (accepted)
            break;
    }
}
