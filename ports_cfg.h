#ifndef _PORTS_CFG_H_
#define _PORTS_CFG_H_
//=============================================================================
enum{pt_pwm,pt_out,pt_inp,pt_adc};
//=============================================================================
typedef struct{
    unsigned char var_idx;    //i.e. ctr_ailerons or any other
    unsigned char bitmask;    //mask if var bitfield or vect idx or array idx
    unsigned char mult;       //multiplier (x0.01) -127[-1.27]..+127[+1.27] (REV)
    signed char   mult_diff;  //differential multiplier (x0.01+1) for pos/neg var value
    unsigned char weight;     //weight to set port value 0..255
    unsigned char port;       //output port number 0...x
    unsigned char type;       //type of port
}__attribute__((packed)) _ports_ctr;
typedef struct {
  signed char   zero;       //output zero shift -127[-1]..+127[+1]
  unsigned char max;        //positive out maximum  0..255[+1]
  unsigned char min;        //negative out minimum 0..255[-1]
}__attribute__((packed)) _ports_pwm;
//=============================================================================
#endif

