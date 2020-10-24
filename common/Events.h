#pragma once

#include <List.h>
#include <do_not_copy.h>

namespace apx {

template<typename T>
class Event : public ListNode<apx::Event<T> *>, private do_not_copy
{
public:
    static void signal(apx::Event<T> *arg = nullptr)
    {
        for (auto i : _list) {
            i->event(arg);
        }
    }
    static inline size_t size() { return _list.size(); }

    typedef T type_t;

protected:
    explicit Event()
    {
        connect();
    }
    ~Event()
    {
        disconnect();
    }
    virtual void event(apx::Event<T> *) = 0;

    int indexOf(apx::Event<T> *e)
    {
        return _list.indexOf(e);
    }
    inline void connect()
    {
        _list.add(this);
    }
    inline void disconnect()
    {
        _list.remove(this);
    }

private:
    static List<apx::Event<T> *> _list;
};

template<typename T>
List<apx::Event<T> *> apx::Event<T>::_list;

} // namespace apx

#define DefineEvent(aname) \
    struct _##aname##_t; \
    using aname = apx::Event<_##aname##_t>
