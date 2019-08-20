//=============================================================
#ifndef escaped_H
#define escaped_H
//==============================================================================
//#include "node.h"
#include <inttypes.h>
#include <string.h>
#include <sys/types.h>
//==============================================================================
#ifndef UART_esc_size
#define UART_esc_size        1024
#endif
//==============================================================================
class Escaped
{
public:
  Escaped()
   : esc_state(0),esc_cnt(0),esc_pos_save(0)
  {
  }
protected:
  virtual uint32_t esc_read(uint8_t *buf,uint32_t sz)=0;
  virtual bool esc_write_byte(const uint8_t v){(void)v;return false;} //write to fifo but don't send

  virtual void escError(void){}
  virtual void escWriteDone(void){}


  uint32_t readEscaped(uint8_t *buf,uint32_t sz)
  {
    uint32_t cnt=readEscaped();
    if(!cnt)return 0;
    if(cnt>sz){
      return 0;
    }
    if(buf!=esc_rx)memcpy(buf,esc_rx,cnt);
    return cnt;
  }

  bool writeEscaped(const uint8_t *buf,uint32_t cnt)
  {
    while(1){
      uint32_t v,crc=0;
      if(!esc_write_byte(0x55))break;
      if(!esc_write_byte(0x01))break;
      while(cnt){
        v=*buf++;
        crc+=v;
        if(!esc_write_byte(v))break;
        if ((v==0x55)&&(!esc_write_byte(0x02)))break;
        cnt--;
      }
      if(cnt)break;
      v=crc&0xFF;
      if(!esc_write_byte(v))break;
      if ((v==0x55)&&(!esc_write_byte(0x02)))break;
      if(!esc_write_byte(0x55))break;
      if(!esc_write_byte(0x03))break;
      escWriteDone();
      return true;
    }
    //error - fifo overflow
    return false;
  }


  //ESC reader
  uint8_t esc_tmp[32];
  uint32_t esc_state;
  uint8_t esc_crc;
  uint16_t esc_cnt,esc_pos_save,esc_cnt_save;
  uint8_t esc_rx[UART_esc_size];      //data from readEscaped()

  uint32_t readEscaped(void) //to local buf esc_rx
  {
    uint32_t rcnt=esc_pos_save?esc_cnt_save:esc_read(esc_tmp,sizeof(esc_tmp));
    if(!rcnt)return 0;
    return readEscaped(rcnt);
  }

  uint32_t readEscaped(uint32_t rcnt) //to local buf esc_rx, from esc_rx
  {
    // 0x55..0x01..DATA(0x55.0x02)..CRC..0x55..0x03
    uint8_t v,*rd=esc_tmp+esc_pos_save;
    esc_pos_save=0;
    while (rcnt--){
      v=*rd++;
      switch (esc_state) {
        case 0:
          if (v==0x55)esc_state=3;
          else break;
          continue;
        case 1: //data
          if (v==0x55) {
            esc_state=2;
            continue;
          }
          case_DATA:
            if (esc_cnt>=sizeof(esc_rx))break;
            esc_rx[esc_cnt++]=v;
            esc_crc+=v;
            continue;
        case 2: //escape
          if (v==0x02) {
            v=0x55;
            esc_state=1;
            goto case_DATA;
          }
          if (v==0x03) {
            if(esc_cnt==0)break; //no data
            esc_cnt--;
            esc_crc-=esc_rx[esc_cnt];
            if ((esc_crc&0xFF)!=esc_rx[esc_cnt])break;
            //frame received...
            esc_state=0;
            if(rcnt){
              esc_pos_save=rd-esc_tmp;
              esc_cnt_save=rcnt;
            }
            return esc_cnt;
          }
          if (v==0x55) {
            esc_state=3;
            continue;
          }
          //fall to case below..
        case 3: // start..
          if (v==0x01) {
            esc_state=1;
            esc_crc=0;
            esc_cnt=0;
            continue;
          }
          break;
      }
      //error
      esc_state=0;
      esc_pos_save=0;
      escError();
    }
    return 0;
  }
};
//==============================================================================
#endif
