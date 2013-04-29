#include "if_base.h"
//==============================================================================
//_if_base::_if_base() {}
bool _if_base::writePacket(const uint8_t *buf,uint cnt){if(cnt)buf++;return true;}
uint _if_base::readPacket(uint8_t *buf,uint sz){if(sz)buf++;return 0;}
bool _if_base::task(void){return false;}
void _if_base::flush(void){}
//==============================================================================
