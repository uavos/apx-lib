#include "MandalaBackport.h"

using namespace mandala::backport;

MandalaBackport::MandalaBackport()
{
    /*    DictMandala *dict = new DictMandala();

    uint8_t sfmt[256];
    uint8_t hfmt[256];

#define MVAR(_type, _name, _descr, _hfmt, _sfmt) \
    sfmt[idx_##_name] = _sfmt; \
    hfmt[idx_##_name] = _hfmt;
#include <Mandala/MandalaTemplate.h>

    for (size_t i = 0; i < sizeof(items) / sizeof(*items); ++i) {
        const item_t &item = items[i];
        if (item.alias != dict->items.at(i).name) {
            apxMsgW() << "alias" << item.alias << dict->items.at(i).name << item.meta.path;
        }
        if (item.fmt_sd != bit && item.fmt_sd != sfmt[dict->items.at(i).id & 0xff]) {
            apxMsgW() << "sfmt" << item.meta.path;
        }
        if (item.fmt_hd != bit && item.fmt_hd != hfmt[dict->items.at(i).id & 0xff]) {
            apxMsgW() << "hfmt" << item.meta.path;
        }
    }*/
}
