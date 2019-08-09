#ifndef Shakti_H
#define Shakti_H
//==============================================================================
#include "Mandala.h"
#include "Mission.h"
#include "conf_apcfg.h"
#include "conf_node.h"
#include "filters.h"
//==============================================================================
class REG_PID
{
public:
    REG_PID(_ft_regPID *acfg)
        : sum(0)
        , out(0)
        , cfg(acfg)
    {
        gain.fill(1);
    }
    void reset(const _var_float v = 0.0);
    _var_float step(_var_float err, _var_float vel, _var_float dt);
    _var_float stepPPI(_var_float err, _var_float vel, _var_float dt);
    _var_float stepPPI_elv(_var_float err, _var_float vel, _var_float dt);
    _var_float stepPPI_ail(_var_float err, _var_float vel, _var_float dt);
    _var_float sum; //sum dt applied when added (to fix float type limit)
    _var_float out;
    _var_vect gain;

private:
    _ft_regPID *cfg;
};
//------------------------------------------------------------------------------
class REG_PI
{
public:
    REG_PI(_ft_regPI *acfg)
        : sum(0)
        , out(0)
        , cfg(acfg)
    {}
    void reset(const _var_float v = 0.0);
    _var_float step(_var_float err, _var_float dt);
    _var_float sum;
    _var_float out;

private:
    _ft_regPI *cfg;
};
//------------------------------------------------------------------------------
class REG_TPI //throttle
{
public:
    REG_TPI(_ft_regPI *acfg)
        : sum(0)
        , out(0)
        , cfg(acfg)
    {}
    void reset(const _var_float v = 0.0);
    _var_float step(_var_float err, _var_float dt);
    _var_float sum;
    _var_float out;

private:
    _ft_regPI *cfg;
};
//------------------------------------------------------------------------------
class REG_P
{
public:
    REG_P(_ft_regP *acfg)
        : out(0)
        , cfg(acfg)
    {}
    _var_float step(_var_float err);
    _var_float out;

private:
    _ft_regP *cfg;
};
//------------------------------------------------------------------------------
class REG_PPI
{
public:
    REG_PPI(_ft_regPPI *acfg)
        : sum(0)
        , out(0)
        , cfg(acfg)
    {}
    void reset(const _var_float v = 0.0);
    _var_float step(_var_float err, _var_float vel, _var_float dt);
    _var_float sum;
    _var_float out;

private:
    _ft_regPPI *cfg;
};
//==============================================================================
class Shakti
{
public:
    Shakti();
    ~Shakti(){};
    //flight mode dispatcher
    void fly(_var_float a_dt);

protected:
    _var_float dt;
    void init();

    // timeout control
    void rc_updated(bool valid);
    void link_updated(bool valid = true);
    void rpm_updated(bool valid);

    // fast controls
    void set_ail(_var_float v);
    void set_elv(_var_float v, bool doMix = true);
    void set_thr(_var_float v, bool doResetPID = true);
    void set_rud(_var_float v, bool doMix = true);
    void set_steering(_var_float v);

    //outer controls
    void set_roll(_var_float v, bool smooth = true, _var_float spd = 0);
    void set_pitch(_var_float v, bool smooth = true, _var_float spd = 0);
    void set_vspeed(_var_float v, bool smooth = true, _var_float spd = 0);
    void set_yaw(_var_float v, bool smooth = true, _var_float spd = 0);

    void set_airspeed(_var_float v);

    void check_config(void); //cfg defaults

    virtual void maneuver(void) = 0; //pure virtual

    //basic maneuvers
    bool flyto(const Point &ne, _var_float sdist = -1); //fly to NED straight, return true if reached
    bool flytoHover(
        const Point &ne,
        _var_float sdist = -1); //fly to NED straight, return true if reached and go to hover mode
    bool flytoLine(const Point &ne,
                   _var_float sdist = -1); //fly to NED by line with rwHDG, return true if reached
    void flytoCircle(const Point &ne,
                     _var_float r,
                     bool bCCW = false,
                     bool adj90 = true); //fly around circle with radius r around NED

    //fly helper functions
    void stabilize();                        //inner loops
    void navigate(bool do_stabilize = true); //outer loops
    void setNE(const Point &ne);             //setup delta, desiredNED, calc dist
    bool goWpt(uint n);                      //set current waypoint and fly
    bool goPi(uint n);                       //set current point of interest and fly
    void ctr_autoflaps(); //_var_float cfgv=0,bool use_cmd=false); //check cfg and control flaps if needed
    _var_float last_autoflaps;

    void starter();

    void lookTo(const Vect &llh); //look camera to llh

    //calculations
    void calcETA(_var_float dWPT, _var_float spd = 0, bool checkHDG = true);

    void safety(); //test for safety procedures (ERS,HOME, etc)
    void ctr_smooth(_var_float *v,
                    _var_float vSet,
                    _var_float speed); //slow down some controls (cmd_roll,cmd_pitch, etc)

    // flight control
    bool ctr_Roll(void);
    bool ctr_Pitch(void);
    bool ctr_Yaw(void);
    bool ctr_Thr(void);
    bool ctr_Airspeed(void);
    bool ctr_Altitude(void);
    bool ctr_VSpeed(void);
    bool ctr_Course(void);
    bool ctr_Line(void);
    bool ctr_Path(void);
    bool ctr_Slip(void);
    bool ctr_Governor(void);
    bool ctr_Thermal(void);
    bool ctr_Taxi(void);
    bool ctr_Brakes(void);
    bool ctr_HoverX(void);
    bool ctr_HoverY(void);
    bool ctr_AirbrkApp(void);
    bool ctr_AirbrkDS(_var_float vDS);

    REG_PID reg_Roll;
    REG_PID reg_Pitch;
    REG_PPI reg_Yaw;
    REG_TPI reg_Thr;
    REG_PID reg_Airspeed;
    REG_P reg_Altitude;
    REG_PID reg_VSpeed;
    REG_PID reg_Course;
    REG_PID reg_Line;
    REG_PID reg_Path;
    REG_PID reg_Slip;
    REG_TPI reg_Governor;
    REG_PI reg_Thermal;
    REG_PID reg_Taxi;
    REG_TPI reg_Brakes;
    REG_PPI reg_HoverX;
    REG_PPI reg_HoverY;
    REG_PI reg_RpmPitch;
    REG_TPI reg_AirbrkApp;
    REG_TPI reg_AirbrkDS;

    //initial procedures after power on
    bool power_on_reset;
    uint counter; //loop counter

    bool doAutoFlaps; //do in navigate()

private:
    _var_float get_gain(_var_float speed, _var_float Ks_low, _var_float Ks_high); //speed coeff
    _var_float get_gain_stab(_var_float Ks_slw);
    _var_float get_gain_rpm(_var_float Ks_low);

    _var_float get_gain_tas();

    //var timeouts
    uint rc_timeout, link_timeout;

    //RPM timeout
    uint rpm_time_s;
    //RPM peak filter
    uint rpm_s, rpmv_s, rpm_cnt;

    bool safety_Ve;
    bool safety_Vs;
    bool safety_VeERS;
    uint32_t wrn_mask; //warnings mask
    uint time_wrn_s;

    //dynamics
    void smooth_cmd_airspeed();
    _var_float cmd_airspeed_dyn;

    //waypoint reached calc
    _var_float last_wpHDG;
    uint wpHDG_cnt;
    bool check_wp_reached(_var_float sdist);

    uint8_t mode_s;
    void checkModeChange(void);

    //ETA
    uint time_eta_s;
    _var_float last_dWPT;
    _var_float dot_dWPT;

    //gimbal
    void control_cam(void);
    void cam_stab_front(void);
    void cam_stab_down(void);
    void cam_stab_roll(void);
    uint cam_mode_s;
    uint time_cam_s;
    //shutter
    uint time_shtr_s;
    Point dshot_ll;

    //state change mon
    _var_float ctr_brake_s;
    _var_float ctr_flaps_s;
    _var_enum gps_jstate_s;

    //shutdown control
    _var_flag sb_s;
    uint time_shdn_s;

    uint8_t starter_stage;
    uint time_starter_s;

    uint time_frate;

protected:
    uint t; //current time

    virtual void onBeforeTakeoff(void);

    //internal
    _var_float venergy_mean;
    _var_float last_brake;

    Mission mission;
    Vect currentPOI_LLH(void);
    void doWaypointActions(void);
    virtual void scr_exec(const char *scr);

    //est
    void calc_stab(void);
    HPF f_stab_turn;

    //restricted areas
    void checkRestrictedAreas(bool reset = false);
    struct
    {
        uint stage;
        uint idx, pidx;
        float minDist;
        Point intersect;
        Point sum, poly_sum;
        Point p1, p2;
        bool isInside, inside_c;
        uint poly_cnt;
        Mission::_item_area *area;
        Mission::_item_area_point *point;
        struct
        {
            float dist;      //distance to area
            Point intersect; //point of contact
            bool isInside;   //inside polygon
            Point sum;       //sum of all forces
        } out;
    } area;

    //primary loops dampers
    DerivativeX<10> d_gyro[3];

    //other
    void print_status_flag(uint8_t var_idx, _var_flag value_prev); //print flag status information
    void print_status_enum(uint8_t var_idx, _var_enum value_prev); //print enum status information
    void print_status_float(uint8_t var_idx, _var_float *value_prev); //print var status information
};
//==============================================================================
#endif
