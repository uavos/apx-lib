#pragma once

#include "MandalaMetaBase.h"

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
struct DictItemInterface
{
};

struct dict_meta_t
{
    const char *name;
    const char *title;
    const char *descr;
    const char *units;
    const char *alias;
    const char *path;

    const uid_t uid : sizeof(uid_t) * 8;
    const uint8_t level : 3;
    const bool group : 1;
    const type_id_t type_id : 4;
    const uint8_t prec : 4;
    const sfmt_id_t sfmt : 4;
};

}; // namespace mandala
