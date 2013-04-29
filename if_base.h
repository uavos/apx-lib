//=============================================================
#ifndef _if_base_H
#define _if_base_H
#include <inttypes.h>
#include <sys/types.h>
//==============================================================================
// standard communication interface object
class _if_base
{
public:
  //_if_base();
  virtual bool writePacket(const uint8_t *buf,uint cnt);
  virtual uint readPacket(uint8_t *buf,uint sz);
  virtual bool task(void);
  virtual void flush(void);
/*virtual bool writePacket(const uint8_t *buf,uint cnt){if(cnt)buf++;return true;}
virtual uint readPacket(uint8_t *buf,uint sz){if(sz)buf++;return 0;}
virtual bool task(void){return false;}
virtual void flush(void){}
*/
};
//==============================================================================
#endif
