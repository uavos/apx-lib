//==============================================================================
#ifndef drv_conf_H
#define drv_conf_H
//==============================================================================
#include "drv_base.h"
#include "drv_files.h"
//==============================================================================
class Conf : public _drv_base
{
public:
  Conf(_node_sn *asn,_fw_info *afw,Files *afiles,bool ais_virtual=false);
  //driver override
  bool service_all(Bus *bus);
  //----------
private:
  uint conf_inf(uint8_t *buf);
  uint conf_dsc_all(uint8_t *buf);
  uint conf_read_all(uint8_t *buf);
  uint conf_write_all(uint8_t *buf);
protected:
  _node_sn *sn;
  _fw_info *fw;
  Files *files;
  bool is_virtual;      //if true - replies node info
  //virtual
  virtual uint conf_cmd(uint8_t *buf);
  virtual uint conf_dsc(uint8_t *buf,uint8_t num);
  virtual uint conf_read(uint8_t *buf,uint8_t num);
  virtual uint conf_write(uint8_t *buf,uint8_t num);
  virtual void conf_reset(void);
  virtual void conf_save(void);
};
//==============================================================================
#define _conf_cmd_NCMD(aname,adescr) \
  *(buf+sz)=apc_##aname; sz++; \
  strcpy((char*)(buf+sz),ASTRINGZ(aname)); sz+=sizeof(ASTRINGZ(aname)); \
  strcpy((char*)(buf+sz),adescr); sz+=sizeof(adescr);
//-------------------------------------
#define _conf_dsc_NCONF(atype,aname,adescr,...) \
  if(num==0){\
    *buf++=ft_##atype; \
    strcpy((char*)buf,ASTRINGZ(aname)); buf+=sizeof(ASTRINGZ(aname));\
    strcpy((char*)buf,adescr); buf+=sizeof(adescr);\
    strcpy((char*)buf,NCARGS(__VA_ARGS__)); buf+=sizeof(NCARGS(__VA_ARGS__));\
    return 1+sizeof(ASTRINGZ(aname))+sizeof(adescr)+sizeof(NCARGS(__VA_ARGS__));\
  }else num--;
//-------------------------------------
#define _conf_read_NCONFA(aconf,atype,aname,asize,adescr,...) \
  if(num==0){\
    memcpy(buf,&(aconf. aname ), sizeof(aconf. aname)); \
    return sizeof(aconf. aname);\
  }else {num--;}
//-------------------------------------
#define _conf_write_NCONFA(aconf,atype,aname,asize,adescr,...) \
  if(num==0){\
    memcpy(&(aconf. aname ), buf, sizeof(aconf. aname)); \
    return sizeof(aconf. aname);\
  }else {num--;}
//==============================================================================
#endif
