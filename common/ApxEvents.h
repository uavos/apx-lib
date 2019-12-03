#pragma once

struct evt_base_t
{
    bool accepted;
    evt_base_t()
        : accepted(false)
    {}
};

#define EVENT_DEF(aname, atype) \
    struct aname : public evt_base_t \
    { \
        atype data; \
        aname(Evt<aname> *src, atype vdata) \
            : evt_base_t() \
            , data(vdata) \
        { \
            src->signal(*this); \
        } \
    };

//-----------------------------------------

template<typename _E>
class Evt
{
public:
    Evt()
        : slot(0)
    {} //{dmsg("signal create\n");}
    class Slot
    {
    public:
        Slot()
            : next_slot(0)
            , esrc(0)
        {} //{dmsg("slot create\n");}
        virtual ~Slot()
        {
            if (esrc)
                disconnect(esrc);
        }
        Slot *next_slot;
        Evt<_E> *esrc;
        virtual void slot(_E &e) = 0;
        void connect(Evt<_E> *src)
        {
            esrc = src;
            src->bind(this);
        }
        void disconnect(Evt<_E> *src) { src->unbind(this); }
    };

private:
    Slot *slot;
    void bind(Slot *bind_slot)
    {
        Slot **el = &slot;
        while (*el)
            el = &((*el)->next_slot);
        *el = bind_slot;
        (*el)->next_slot = 0; //chain tail
    }
    void unbind(Slot *bind_slot)
    {
        Slot **el_prev = 0;
        Slot **el = &slot;
        while (*el && *el != bind_slot) {
            el_prev = el;
            el = &((*el)->next_slot);
        }
        if (!*el)
            return;
        if (*el == slot) {
            slot = (*el)->next_slot;
        } else {
            (*el_prev)->next_slot = (*el)->next_slot;
        }
    }

public:
    void signal(_E e)
    {
        e.accepted = false;
        Slot **el = &slot;
        while (*el) {
            (*el)->slot(e);
            if (e.accepted)
                break;
            el = &((*el)->next_slot);
        }
    }
};
