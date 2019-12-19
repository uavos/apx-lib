#pragma once

#include "Xdb.h"
#include "XdbPub.h"
#include "XdbSub.h"

#include <containers/OrderedList.hpp>

namespace xdb {

class XdbManager
{
public:
    explicit XdbManager();
    static XdbManager *instance() { return _instance; }

    static void initialize();

    static void publish(uid_t uid, const void *data, uint16_t size);

    static void subscribe(XdbSub *n);
    static void unsubscribe(XdbSub *n);

    static void advertise(XdbPub *n);
    static void unadvertise(XdbPub *n);

private:
    static XdbManager *_instance;

    List<XdbPub *> _pubs;
    OrderedList<XdbSub *> _subs;
};

} // namespace xdb
