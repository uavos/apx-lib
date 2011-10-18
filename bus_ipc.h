#ifndef bus_ipc_H
#define bus_ipc_H
#include <inttypes.h>
#include <sys/msg.h>
//==============================================================================
// messages from bus
#define _bus_msg_size     1024
typedef struct{
  long    type;   //must be long (used by filter)
  uint8_t data[_bus_msg_size];
}_bus_msg;
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

