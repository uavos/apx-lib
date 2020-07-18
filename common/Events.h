#pragma once

#include <List.h>
#include <do_not_copy.h>

namespace apx {

template<typename T>
class Event : public ListNode<Event<T> *>, private do_not_copy
{
public:
    static void signal(Event<T> *arg = nullptr)
    {
        for (auto i : _list) {
            i->event(arg);
        }
    }

protected:
    explicit Event()
    {
        _list.add(this);
    }
    ~Event()
    {
        _list.remove(this);
    }
    virtual void event(Event<T> *) = 0;

private:
    static List<Event<T> *> _list;
};

template<typename T>
List<Event<T> *> Event<T>::_list;

} // namespace apx

#define DefineEvent(name) \
    struct evt_##name \
    {}; \
    using name = apx::Event<evt_##name>
