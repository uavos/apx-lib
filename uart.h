//=============================================================
#ifndef uart_H
#define uart_H
//=============================================================
#include <inttypes.h>
#include <termios.h>
//=============================================================
class Uart
{
public:
  Uart();
  ~Uart();

  bool open(const char *portname="",int baudrate=0,const char *name="",int timeout=1);
  void close();
  bool isOpen(void);
  int handle(void);

  void write(const uint8_t v);
  void write(const uint8_t *buf,uint cnt);
  void writeEscaped(const uint8_t *tbuf,uint dcnt);

  uint read(uint8_t *buf,uint cnt);
  uint readEscaped(uint8_t *buf,uint max_len);

  uint8_t getCRC(const uint8_t *buf,uint cnt);

  unsigned int getRxCnt(void);
  unsigned int getTxCnt(void);
  uint isBusy(void);
  void flush(void);

private:
  int   fd;
  const char    *name;
};
//=============================================================
#endif
