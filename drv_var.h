//=============================================================
#ifndef _drv_var_H
#define _drv_var_H
#include "hardware.h"
#include "drv_base.h"
#include "MandalaCore.h"
//==============================================================================
class VarDrv : public _drv_base
{
public:
private:
  //driver override
  bool var_update(Bus *bus)
  {
    extern MandalaCore var;
    var.extract(bus->packet.data,bus->packet_cnt-bus_packet_size_hdr,bus->packet.id);
    return false;
  }
};
//==============================================================================
#endif
