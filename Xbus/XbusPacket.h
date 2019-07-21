#pragma once
#include "XbusBase.h"

class XbusPacket: public XbusBase
{
public:
    XbusPacket(uint8_t *packet_ptr);

    typedef uint8_t pid_t;

    enum {
        size_packet = 1024,
    };

    /*enum {
        pid_service     =0,     //Service packet <node_sn>,<cmd>,<data..>
        //special protocols
        pid_xpdr,               //Transponder data <xpdr>
        pid_ident,              //Identification <ident>
        pid_dlink,              //Datalink <squawk>,<var_idx>,<data..>

        pid_ping        =8,     //Ping packet, no reply expected
        pid_sim,                //Simulator data <var_idx>,<data..>
        pid_hid,                //HID data <var_idx>,<data..>
        pid_jsexec,             //Execute JS script <script text>

        //mandala
        pid_downstream  =16,    //Downlink stream <stream>
        pid_uplink,             //Uplink wrapped packet <var_idx>,<data..>
        pid_mission,            //Mission data <packed mission>
        pid_set,                //Set component <var_idx>,<msk>,<float/byte>
        pid_data,               //Port serial data <port_id>,<data..>
        pid_ldata,              //Port data local <port_id>,<data..>
        pid_can,                //Auxilary CAN data <ID32 b31=IDE>,<data..>
        pid_vmexec,             //Execute VM script <@function>
        //other & temporary
        pid_prio,               //Priority assigned var <nodeID>,<prio>,<var_idx>,<data..>
        pid_active,             //Autopilot active flag
        pid_video,              //Video stream <data>

        pid_formation,          //Formation flight data <packed data>
        pid_vor,                //VOR beacon <packed data>
    };*/


    pid_t pid() const;
    void setPid(const pid_t v);
};
