//=============================================================
#ifndef _drv_base_H
#define _drv_base_H
#include "bus.h"
//==============================================================================
// standard driver interface object
class _drv_base
{
public:
  _drv_base() {valid=false;};

  virtual void reset_config(void){};    //reset config command request
  virtual bool check_config(void){return true;}; //check if config needs to be reset

  virtual void task(Bus *bus){};        //some task in main cycle
  virtual bool service(Bus *bus){return false;}; //service addressed comand
  virtual bool service_all(Bus *bus){return false;}; //service comand any address
  virtual bool var_request(Bus *bus){return false;}; //zero var packet
  virtual bool var_update(Bus *bus){return false;}; //var received

  bool valid;
};
//==============================================================================
#endif
