#pragma once
#include "XbusPacket.h"

#include <array>

class XbusNode: public XbusPacket
{
public:
    XbusNode(uint8_t *packet_ptr);

    typedef std::array<uint8_t,12>  guid_t; //global unique node id
    typedef uint8_t                 cmd_t;  //service command

    // system service commands (service packets)
    typedef enum{
      apc_ack=0,    //acknowledge - sent back in response to commands
      //------------------
      //system commands
      apc_search,   //return just sn (broadcast)

      apc_info=8,   //return _node_info
      apc_nstat,    //return _node_name + _node_status
      apc_reboot,   //reset/reboot node
      apc_mute,     //stop sending data (sensors)
      apc_reconf,   //reset all conf to defaults
      apc_power,    //return _node_power
      apc_mem,      //print node memory usage
      apc_shock,    //shock test

      //------------------
      //conf
      apc_conf_inf=32,      //return _conf_inf structure
      apc_conf_cmds,        //return node commands descrptor
      apc_conf_dsc,         //return <num> parameter descriptor
      apc_conf_read,        //return parameter <num>
      apc_conf_write,       //save parameter <num>,<data>

      //------------------
      //script storage
      apc_script_file,     //get or set file info [<_flash_file>], re: [<_flash_file>]
      apc_script_read,     //read data <_flash_data_hdr>, re: <_flash_data>
      apc_script_write,    //write data <_flash_data>, re: <_flash_data_hdr>

      apc_msg=64,   //text message
      //------------------
      //user commands
      apc_user=128,
      apc_loader=0xFF
    } comands_t;

    typedef enum {
      ldc_init=0,   // start loader, re: <_flash_file> =flash size
      ldc_file,     // set file info <_flash_file>, re: <_flash_file>
      ldc_write     // write data <_flash_data>, re: <_flash_data_hdr>
    } comands_loader_t;


    struct Header {
        guid_t      guid;
        cmd_t       cmd;
    };

    void read(Header &d);
    void write(const Header &d);

private:
    uint8_t *_hdr;
};
