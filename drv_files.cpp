//==============================================================================
#include <stdio.h>
#include <string.h>
#include "drv_files.h"
#include "node_conf.h"
#include "apcfg.h"
#include "Mandala.h"
extern Mandala var;
//==============================================================================
Files::Files()
: writing(false),file_op(0)
{
}
//==============================================================================
void Files::saveFile(uint file_idx)
{
  file_op|=1<<file_idx;
}
//==============================================================================
bool Files::loadFile(uint file_idx)
{
  uint rcnt=read(file_idx);
  if((!rcnt)||(fbuf.file.size!=(rcnt-file_hdr_size))||(fbuf.file.crc!=fcrc()))
    return false;
  //file loaded
  switch(file_idx){
    case file_conf:
      if(fbuf.file.size!=sizeof(conf))return false;
      memcpy(&conf,fbuf.file.data,sizeof(conf));
      return true;
    case file_ap:
      if(fbuf.file.size!=sizeof(apcfg))return false;
      memcpy(&apcfg,fbuf.file.data,sizeof(apcfg));
      return true;
    case file_fp:
      return var.extract(fbuf.file.data,fbuf.file.size,idx_flightplan);
  }
  return false;
}
//==============================================================================
uint8_t Files::fcrc(void)
{
  uint cnt=fbuf.file.size;
  const uint8_t *ptr=fbuf.file.data;
  uint8_t v=cnt;
  while(cnt--)v^=*ptr++;
  return v;
}
//==============================================================================
//==============================================================================
void Files::flush(void)
{
  while(file_op)task(NULL);
}
//==============================================================================
uint Files::read(uint file_idx)
{
  return 0;
}
//==============================================================================
bool Files::write_task(uint file_idx)
{
  return true;
}
//==============================================================================
bool Files::do_write(uint file_idx)
{
  if(writing)return write_task(file_idx);
  //prepare file buffer
  switch(file_idx){
    case file_conf:
      memcpy(fbuf.file.data,&conf,sizeof(conf));
      fbuf.file.size=sizeof(conf);
      fbuf.file.crc=fcrc();
      break;
    case file_ap:
      memcpy(fbuf.file.data,&apcfg,sizeof(apcfg));
      fbuf.file.size=sizeof(apcfg);
      fbuf.file.crc=fcrc();
      break;
    case file_fp:
      fbuf.file.size=var.archive(fbuf.file.data,sizeof(fbuf.file.data),idx_flightplan);
      fbuf.file.crc=fcrc();
      break;
  }
  writing=true;
  return write_task(file_idx);
}
//==============================================================================
//==============================================================================
void Files::task(Bus *bus)
{
  //if(!valid)return;
  uint32_t mask=1;
  for(uint i=0;file_op&&i<fileCnt;i++){
    if(file_op&mask){
      if(do_write(i)){
        file_op&=~mask;
        writing=false;
      }else break;
    }
    mask<<=1;
  }
}
//==============================================================================
bool Files::var_update(Bus *bus)
{
  if(bus->packet.id==idx_flightplan) saveFile(Files::file_fp);
  return false;
}
//==============================================================================
