#ifndef bus_ipc_H
#define bus_ipc_H
#include <inttypes.h>
#include <sys/msg.h>
//==============================================================================
// messages from bus
typedef struct{
  long    type;   //must be long (used by filter)
  uint8_t data[1024];
}_bus_msg;
#define _bus_msg_size     (sizeof(_bus_msg)-offsetof(_bus_msg,data))
enum{
  mtypeBusAll=0,  //reserved for receive all filter
  mtypeBusTx,     //transmit to uart bus
  mtypeBusRx,     //received from uart bus
  mtypeUdpTx,     //transmit to gcu (UDP)
  mtypeSimTx,     //transmit to Simulator (UDP)
};
//------------------------------------------------------------------------------
static inline int getMsgId(void){return msgget(ftok("/etc/fstab",'a'),0644|IPC_CREAT);};
//==============================================================================
#endif

