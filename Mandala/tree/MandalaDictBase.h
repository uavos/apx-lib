#pragma once

#include <cinttypes>
#include <sys/types.h>

namespace mandala {

template<class MandalaMeta>
class group : public MandalaMeta
{
public:
    void subscribe()
    {
    }
};

template<class MandalaMeta>
class value : public MandalaMeta
{
public:
    void subscribe()
    {
    }
};

template<class MandalaMeta>
struct DictItemInterface
{
};

struct dict_meta_t
{
    const char *name;
    const char *title;
    const char *descr;

    const uid_t uid : sizeof(uid_t) * 8;
    const uint8_t level : 4;
};

}; // namespace mandala
