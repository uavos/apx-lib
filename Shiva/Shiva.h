#ifndef SHIVA_H
#define SHIVA_H
#include "Shakti.h"
#include "kalman.h"
//==============================================================================
//expanded procedures
namespace LANDING_stage {
  enum{
    init=0,
    TA,TA_do,
    TAapp,TAapp_do,
    App,App_do,
    Flare,Flare_do,Flare_done,
    Touchdown,Touchdown_do,
    Run,Run_do,Run_done,
    Cancel=100,Cancel_do,Cancel_done,
    Finished=250,Finished_do,Finished_done,
  };
}
namespace TAKEOFF_stage {
  enum{
    init=0,
    Taxi,
    Start,Start_do,
    Run,Run_do,
    Liftoff,Liftoff_do,
    Climbing,Climbing_do,
    Finished,
    Cancel=100,
    WaitFP,WaitFP_do,WaitFP_done,
    WaitPOS,WaitPOS_do,WaitPOS_done,
  };
}
//==============================================================================
//==============================================================================
class Shiva : protected Shakti
{
public:
  Shiva();
  // override
  void maneuver(void);

private:
  // methods
  void EMG(void);
  void RPV(void);
  void UAV(void);
  void WPT(void);
  void HOME(void);
  void STBY(void);
  void TAXI(void);
  void TAKEOFF(void);
  void LANDING(void);

  uint TAKEOFF_fixedwing(uint stage);
  uint TAKEOFF_helicopter(uint stage);
  uint TAKEOFF_blimp(uint stage);
  uint TAKEOFF_boat(uint stage);

  uint LANDING_fixedwing(uint stage);
  uint LANDING_helicopter(uint stage);
  uint LANDING_blimp(uint stage);
  uint LANDING_boat(uint stage);

  //helper vars for some modes..
  bool ers_landing;
  uint8_t prev_rwidx,prev_twidx;
  uint8_t prev_sgps;
  Point rwNE;           // start of runway
  Point appNE;          // start point for runway approach
  _var_float appLength;
  bool rwLeft;          // true if left turn runway
  _var_float pitch_s;   // saved pitch angle for next stage
  _var_float pitch_s2;  // saved pitch angle for next stage
  _var_float pitch_s3;  // saved pitch angle for next stage
  _var_float speed_s;   // saved speed for next stage
  _var_float alt_corr;  // altitude correction in command touchdown mode
  uint time_s;          // general stage timeout
  int alt_speech;       // landing say altitude
  union{
    uint time_thr_s;    // takeoff throttle delay
    uint time_stage_s;  // takeoff throttle delay
    uint sub_stage;     // sub stage used on landing
  };
  //taxi
  bool setup_taxiway();


  //venergy landing
  bool adjust_landing_circle();
  void get_landing_length(_var_float gnd_altitude);
  Point find_landing_tangent(void);
  void setDelta(_var_float delta);
  _var_float vLE_total; // total path safe to glide
  _var_float vLE_est;   // estimated path to glide
  _var_float app_Kwt;   // approach wind triangle K
  uint turnsVE;         // number of circles
  uint time_VE;
  bool last_VE_result;
  _var_float rwHDG;     // current runway heading
  Kalman delta_kf;

  //circle params
  Point         cNE;    // circle center
  _var_float    cR;     // circle radius
  _var_float    cSpan;  // arc span
  _var_float    aLen;   // initial appLength
};
//==============================================================================
#endif
