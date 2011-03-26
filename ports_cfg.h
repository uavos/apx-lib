#ifndef _PORTS_CFG_H_
#define _PORTS_CFG_H_
//=============================================================================
// Ports configuration struct. must be defined in node's _cfg.
// _ports_ctr[] must be first in _cfg, _ports_pwm[] always second

//type of port
enum{pt_pwm,pt_out,pt_inp,pt_adc};
//=============================================================================
typedef struct{
    unsigned char var_idx;    //i.e. ctr_ailerons or any other
    unsigned char bitmask;    //mask if var bitfield or vect idx or array idx
    signed char   mult;       //multiplier (x0.1) -127[-12.7]..+127[+12.7]
    signed char   diff;       //differential multiplier (x0.01+1) for pos/neg var value
    unsigned char weight;     //weight to set port value 0..255
    unsigned char port;       //output port number 0...x
    unsigned char type;       //type of port
}__attribute__((packed)) _ports_ctr;
typedef struct {
  signed char   zero;         //pwm zero shift -127[-1]..+127[+1]
  signed char   max;          //pwm maximum -127[-1]..+127[+1]
  signed char   min;          //pwm minimum -127[-1]..+127[+1]
}__attribute__((packed)) _ports_pwm;
//=============================================================================
#endif

