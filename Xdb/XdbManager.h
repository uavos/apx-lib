#pragma once

#include "Xdb.h"
#include "XdbPub.h"
#include "XdbSub.h"

#include <containers/OrderedList.hpp>

namespace xdb {

class XdbManager
{
public:
    static void publish(uid_t uid, const void *data, uint16_t size);

    static void subscribe(XdbSub *n);
    static void unsubscribe(XdbSub *n);

    static void advertise(XdbPub *n);
    static void unadvertise(XdbPub *n);

private:
    static List<XdbPub *> _pubs;
    static OrderedList<XdbSub *> _subs;
};

} // namespace xdb
