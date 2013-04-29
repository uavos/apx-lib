//=============================================================
#ifndef bus_H
#define bus_H
//==============================================================================
#include <inttypes.h>
#include <sys/types.h>
#include "if_base.h"
#include "node.h"
#define BUS_MAX_DRIVERS         32
#define BUS_MAX_INTERFACES      4
#define BUS_MAX_DOWNLINKS       4
//==============================================================================
class _drv_base;
class Bus
{
public:
  Bus();
  void add_interface(_if_base *iface);
  void add_downlink(_if_base *iface); //fill wrap idx_uplink and filter autosend
  void add_driver(_drv_base *drv);

  void flush(void);

  bool check_packet_sn(_node_sn *sn,bool force_sn=false);

  bool write(uint8_t id,const uint8_t *buf,uint cnt);
  bool write_tagged(uint8_t tag,const uint8_t *buf,uint cnt);
  bool write_srv(_node_sn *sn,uint8_t cmd,const uint8_t *buf,uint cnt);
  bool write_srv(uint8_t cmd,const uint8_t *buf,uint cnt);
  bool write_ldr(uint8_t cmd,const uint8_t *buf,uint cnt);
  bool write_raw(const uint8_t *buf,uint cnt);
  bool write_ack(void);
  bool write_ack(uint8_t cmd);
  bool write_packet(uint data_cnt); //local packet

  void debug(const char *string);

  //task
  bool task();

  //read
  uint read(void);
  void unwrap_packet(void);
  uint process_packet(void);

  void default_task(void);

  _bus_packet packet;
  uint packet_cnt; //including header
  bool packet_broadcast; //set after 'read' if was broadcast packet (sn=000000..)
  uint8_t packet_wrapped;       //received packet wrapped idx
  uint8_t packet_tag;           //received packet tag (if any)

  //drivers support
  bool drivers_data(void);
  void drivers_task(void);
  void drivers_reset_config(void);

  bool mute;
private:
  _if_base *if_obj[BUS_MAX_INTERFACES]; //communication device
  uint8_t if_cnt;
  _if_base *dl_obj[BUS_MAX_DOWNLINKS]; //downlink device
  uint8_t dl_cnt;
  _drv_base *drv_obj[BUS_MAX_DRIVERS]; //driver
  uint8_t drv_cnt;
};
//==============================================================================
#endif
