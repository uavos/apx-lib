//==============================================================================
#ifndef drv_files_H
#define drv_files_H
//==============================================================================
#include "drv_base.h"
#define file_hdr_size  3        //(sizeof(fbuf.file)-sizeof(fbuf.file.data))
//==============================================================================
class Files : public _drv_base
{
public:
  Files();
  //driver override
  void task(Bus *bus);
  bool var_update(Bus *bus);
  //-------------------------
  enum{file_conf,file_ap,file_fp, fileCnt};
  void saveFile(uint file_idx);
  bool loadFile(uint file_idx);
  virtual void flush(void);
  //-------------------------
  typedef struct{
    uint8_t addrH; //used for memory addressing
    uint8_t addrL; //used for memory addressing
    struct{
      uint16_t size; //data size
      uint8_t  crc;
      uint8_t  data[1024];
    }file;
  }__attribute__((packed)) _fbuf;
  _fbuf fbuf;
protected:
  virtual uint read(uint file_idx); //synchronous fill fbuf.file, return read bytes cnt
  virtual bool write_task(uint file_idx); //write fbuf.file, async, return is_finished
private:
  uint8_t fcrc(void);
  bool writing;
  uint32_t file_op;
  bool do_write(uint file_idx);
};
//==============================================================================
#endif
