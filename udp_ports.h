#ifndef udp_ports_H
#define udp_ports_H
//==============================================================================
#define UDP_PORT_TELEMTRY       9000    //shiva: telemetry downlink/uplink
#define UDP_PORT_GCU            9001    //gcu: telemetry gate downlink/uplink
#define UDP_PORT_SIM            9002    //simulator: imu/servo

#define UDP_HOST_GCU            "10.1.1.100"    //machine where 'GCU' runs
#define UDP_HOST_UAV            "10.1.1.1"      //machine where 'shiva' runs
#define UDP_HOST_SIM            "10.1.1.101"    //machine where 'xplane' runs
#define UDP_HOST_DATA           "10.1.1.110"    //fwd packets on port GCU (user)
//==============================================================================
#endif
