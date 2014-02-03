#ifndef udp_ports_H
#define udp_ports_H
//==============================================================================
#define UDP_MODEM_TX            9010    //modem tx (send to UAV)

#define UDP_PORT_TELEMTRY       9000    //shiva: telemetry downlink/uplink
#define UDP_PORT_SIM            9002    //simulator: imu/servo
#define UDP_PORT_DSP            9004    //simulator: display
#define UDP_PORT_HIL            9006    //simulator: hil

#define UDP_HOST_GCU            "10.1.1.100"    //machine where 'GCU' runs
#define UDP_HOST_UAV            "10.1.1.1"      //machine where 'shiva' runs
#define UDP_HOST_SIM            "10.1.1.101"    //machine where 'xplane' runs
//==============================================================================
#endif
