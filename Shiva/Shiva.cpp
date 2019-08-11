#include "Shiva.h"

#include "time_ms.h"

#include <cmath>
#include <dmsg.h>
//==============================================================================
extern Mandala var;
//==============================================================================
Shiva::Shiva()
    : Shakti()
{
    prev_rwidx = prev_twidx = prev_sgps = 0;
}
//==============================================================================
void Shiva::maneuver(void)
{
    //mode
    switch (var.mode) {
    case mode_EMG:
        EMG();
        break;
    case mode_RPV:
        RPV();
        break;
    case mode_UAV:
        UAV();
        break;
    case mode_WPT:
        WPT();
        break;
    case mode_HOME:
        HOME();
        break;
    case mode_STBY:
        STBY();
        break;
    case mode_TAXI:
        TAXI();
        break;
    case mode_TAKEOFF:
        TAKEOFF();
        break;
    case mode_LANDING:
        LANDING();
        break;
    default:
        dmsg("Wrong flight mode %u\n", (uint32_t) var.mode);
        var.mode = mode_HOME;
    }
}
//==============================================================================
void Shiva::EMG(void)
{
    if (!var.stage) {
        var.stage = 1;
        var.power |= power_agl;
    }
    RPV();
    switch (apcfg.type) {
    case type_airplane:
    case type_helicopter:
        set_ail(var.rc_roll);
        set_elv(var.rc_pitch, false);
        set_steering(var.rc_yaw);
        set_rud(var.rc_yaw, false);
        set_thr(var.rc_throttle);
        if (apcfg.type == type_helicopter)
            var.ctr_collective = var.rc_throttle * 2.0 - 1.0;
        break;
    case type_blimp:
        var.ctr_collective = var.rc_pitch;
        set_elv(var.rc_pitch, false);
        set_steering(var.rc_roll);
        set_rud(var.rc_roll, false);
        set_thr(var.rc_throttle);
        break;
    case type_boat:
        set_steering(var.rc_roll);
        set_rud(var.rc_roll, false);
        set_thr(var.rc_throttle);
        break;
    }
}
//==============================================================================
void Shiva::RPV(void)
{
    if ((!var.stage) || (var.stage > 2)) { //reset cmd_NED
        var.stage = 1;
        set_vspeed(0, false);
        //setNE(var.NED[0],var.NED[1]);
    }
    var.calc(); //calc distHome
    calcETA(var.dHome);
    switch (apcfg.type) {
    case type_airplane:
        if (apcfg.frm_mode == frm_mode_wingL || apcfg.frm_mode == frm_mode_wingR) {
            var.rc_throttle = var.ctr_throttle;
            var.rc_yaw = var.ctr_rudder;
        } else {
            set_roll(0);
            set_pitch(0); //set by rc adjust
        }
        ctr_autoflaps();
        stabilize();
        if (var.stage <= 1) { //auto throttle if stage changed
            set_thr(var.rc_throttle);
        } else {
            ctr_Thr();
            ctr_Governor();
        }
        break;
    case type_helicopter:
        var.cmode |= cmode_hover;
        set_roll(0, false);  //set by rc adjust
        set_pitch(0, false); //set by rc adjust
        set_yaw(var.cmd_course);
        stabilize();
        if (var.stage <= 1) { //auto throttle if stage changed
            var.ctr_collective = var.rc_throttle * 2.0 - 1.0;
            reg_VSpeed.reset(var.ctr_collective * 100.0);
            var.cmd_altitude = var.altitude;
        } else {
            ctr_Altitude();
            ctr_VSpeed();
        }
        if (!ctr_Governor())
            set_thr(var.rc_throttle);
        break;
    case type_blimp:
        set_vspeed(var.rc_pitch * apcfg.Altitude.Lo, false);
        ctr_VSpeed(); //pitch
        stabilize();
        set_steering(var.rc_roll);
        set_rud(var.rc_roll, false);
        set_thr(var.rc_throttle);
        break;
    case type_boat:
        set_steering(var.rc_roll);
        set_rud(var.rc_roll, false);
        set_thr(var.rc_throttle);
        break;
    }
}
//==============================================================================
void Shiva::UAV(void)
{
    if ((!var.stage) || (var.stage > 2)) { //reset cmd_NED
        var.stage = 1;
        setNE(var.pos_NE);
        if (apcfg.type == type_helicopter)
            var.cmode |= cmode_hover;
    } else
        var.calc(); //calc distHome
    navigate();
    calcETA(var.dHome);
    if (var.stage <= 1) { //auto throttle if stage changed
        if (apcfg.type == type_helicopter) {
            var.ctr_collective = var.rc_throttle * 2.0 - 1.0;
            reg_VSpeed.reset(var.ctr_collective * 100.0);
            var.cmd_altitude = var.altitude;
        } else {
            set_thr(var.rc_throttle);
        }
    }
}
//==============================================================================
void Shiva::WPT(void)
{
    if (!var.stage) {
        goWpt(0);
        return;
    }
    bool bReached = false;
    switch (var.mtype) {
    case mtype_hdg:
        bReached = flyto(var.cmd_NE);
        break;
    case mtype_line:
        bReached = flytoLine(var.cmd_NE);
        if ((!bReached) && (std::abs(var.boundAngle(var.tgHDG - var.wpHDG)) > 90))
            var.mtype = mtype_hdg;
        break;
    }
    navigate();
    calcETA(var.dWPT);
    if (bReached) {
        // Waypoint reached
        dmsg("Waypoint passed (#%u)\n", var.wpidx + 1);
        doWaypointActions();
        if (var.mode == mode_WPT) //action didn't change the mode
            goWpt(var.wpidx + 1);
    }
}
//==============================================================================
void Shiva::HOME(void)
{
    switch (apcfg.type) {
    case type_airplane:
        flyto(0, 0);
        navigate();
        break;
    case type_helicopter:
        var.tgHDG = var.cmd_course;
        flytoHover(Point(0, 0), apcfg.wpt_snap);
        navigate();
        break;
    case type_blimp:
        if (flyto(0, 0)) {
            if (var.cmd_airspeed != 0)
                var.cmd_airspeed = 0;
        } else {
            if (var.cmd_airspeed == 0)
                var.cmd_airspeed = apcfg.spd_cruise / 2.0;
        }
        navigate();
        break;
    case type_boat:
        var.mode = mode_UAV;
        break;
    }
}
//==============================================================================
void Shiva::STBY(void)
{
    //if(apcfg.type==type_blimp) var.mode=mode_UAV;
    if (var.stage == LANDING_stage::Cancel) {
        var.stage = 1;
        time_VE = t;
        turnsVE = 0;
    } else if ((!var.stage) || (var.stage > 2)) {
        var.stage = 1;
        if (var.cmd_altitude < apcfg.altitude) //climb if too low
            var.cmd_altitude = apcfg.altitude;
        if (mission.current.pi) { //go POI
            setNE(var.llh2ne(currentPOI_LLH()));
        } else if (mission.current.wp) { //wp action
            setNE(var.cmd_NE);
        } else { //STBY mode
            setNE(var.pos_NE);
            mission.current.timeS = 600;
        }
        var.loops = mission.current.loops;
        if (var.loops == 0)
            var.loops = 1;
        if (mission.current.turnR == 0)
            mission.current.turnR = apcfg.turnR;

        if (apcfg.type == type_helicopter) {
            if (var.dWPT <= apcfg.wpt_snap && var.gSpeed <= apcfg.HoverX.Lpp * 2.0)
                var.cmode |= cmode_hover;
            //only timeout (no loops for helicopter)
            if (!mission.current.timeS)
                mission.current.timeS = 600;
            var.loops = 0;
        }

        //camera track
        Point ll(var.ne2ll(var.cmd_NE));
        _var_float valt = (mission.current.pi && (mission.current.pi->hmsl != 0))
                              ? mission.current.pi->hmsl
                              : var.home_pos[2];
        var.cam_tpos = Vect(ll[0], ll[1], valt);

        //loops counter
        cSpan = var.wpHDG;
        time_VE = t;
        turnsVE = 0;
    } //init
    if (apcfg.type == type_helicopter) {
        //adjust position
        Point adj;
        const _var_float dw = 0.1;
        if (var.rc_roll > dw)
            adj[1] = var.rc_roll - dw;
        else if (var.rc_roll < -dw)
            adj[1] = var.rc_roll + dw;
        if (var.rc_pitch > dw)
            adj[0] = -var.rc_pitch + dw;
        else if (var.rc_pitch < -dw)
            adj[0] = -var.rc_pitch - dw;
        if (!adj.isNull()) {
            adj = var.rotate(adj, -var.cmd_theta[2]);
            var.cmd_NE = var.pos_NE + adj;
            time_VE = t; //reset time while moving point
        }
        if (!(var.cmode & cmode_hover)) {
            var.tgHDG = var.cmd_course;
            flytoHover(var.cmd_NE, apcfg.wpt_snap);
            time_VE = t; //reset time while flying to point
        } else {
            var.calc();
            if ((var.cmode & cmode_hover) && var.dWPT > (apcfg.wpt_snap * 1.5)) {
                var.cmode &= ~cmode_hover;
                var.cmd_airspeed = apcfg.spd_cruise;
            }
        }
    } else { //airplane
        var.turnR = mission.current.turnR + var.rwAdj * (std::abs(mission.current.turnR) * 0.5);
        flytoCircle(var.cmd_NE, std::abs(var.turnR), var.turnR < 0);
        //thermals
        if (var.turnR == 0 && (!mission.current.pi) && apcfg.type == type_airplane
            && ctr_Thermal()) {
            ctr_Altitude();
            ctr_Airspeed();
            ctr_Thr();
            ctr_Governor();
            ctr_autoflaps();
            stabilize();
            return;
        }
        //count loops
        if (std::abs(var.boundAngle(cSpan - var.wpHDG)) >= 90) {
            cSpan = var.wpHDG;
            turnsVE++;
            if (turnsVE >= 4) {
                turnsVE = 0;
                if (var.loops > 0)
                    var.loops--;
            }
        }
    }
    navigate();
    //check if done
    bool bDone = var.loops == 0;
    if (bDone && mission.current.timeS)
        bDone = (t - time_VE) > (mission.current.timeS * 1000);
    if (bDone || var.stage > 1) {
        goWpt(var.wpidx);
        return;
    }
    //ETA
    if (var.dWPT > (var.turnR * 3.0)) {
        calcETA(var.dWPT);
    } else if (mission.current.timeS) {
        uint32_t xt = (t - time_VE) / 1000;
        if (mission.current.timeS > xt)
            var.ETA = mission.current.timeS - xt;
        else
            var.ETA = 0;
    } else { //loops
        _var_float dist = std::abs(var.turnR) * (2.0 * PI);
        var.dWPT = dist * var.loops
                   + dist * (360.0 - (std::abs(var.boundAngle(cSpan - var.wpHDG)) + turnsVE * 90.0))
                         / 360.0;
        calcETA(var.dWPT, var.cmd_airspeed * var.cas2tas, false);
    }
}
//==============================================================================
void Shiva::TAXI(void)
{
    if (prev_twidx != var.twidx) { //protect taxiway change
        if (var.stage < 2)
            var.stage = 0;
        else {
            dmsg("Select taxiway #%u.\n", var.twidx + 1);
            var.stage = 3;
        }
        prev_twidx = var.twidx;
    }
    if ((!prev_sgps) && (var.status & status_gps)) { //protect home pos change
        prev_sgps = 1;
        var.stage = 0;
    }
    switch (var.stage) {
    case 0: {
        var.stage++;
        var.ctrb &= ~ctrb_ers;
        var.ctrb &= ~ctrb_rel;
        var.power &= ~power_agl;
        var.sw |= sw_taxi;
        var.rwAdj = 0;
        set_roll(0, false);
        set_pitch(0, false);
        set_vspeed(0, false);
        var.rc_throttle = 0;
        var.ctr_brake = 1;
        var.cmd_airspeed = apcfg.taxi_gSpeed;
        var.ctr_flaps = 0;
        set_steering(var.rc_roll);
        set_rud(var.rc_roll, false);
        reg_Roll.reset();
        reg_Pitch.reset();
        reg_Brakes.reset();
        reg_Taxi.reset();
        reg_Yaw.reset();
        //display only
        if (setup_taxiway())
            setNE(appNE);
    } break;
    case 1: {
        var.calc();
        ctr_Taxi();
        set_steering(var.rc_roll);
        set_rud(var.rc_roll, false);
    } break;
    case 2: {
        dmsg("Auto taxi\n");
        var.stage++;
    } break;
    case 3: {
        if (!setup_taxiway()) {
            if (var.twidx > 0) {
                var.twidx = 0;
                dmsg("Taxi pattern finished\n");
            }
            var.stage = 0;
            return;
        }
        dmsg("Taxiway #%u. HDG:%i\n", var.twidx + 1, (int) var.boundAngle360(rwHDG));
        setNE(appNE);
        var.stage++;
    } break;
    case 4: {
        var.calc();
        set_roll(0, false);
        set_pitch(0);
        stabilize();
        reg_Roll.reset();
        reg_Pitch.reset();
        ctr_Taxi();
        ctr_Yaw();
        ctr_Brakes();
        var.delta = var.distance(var.pos_NE - rwNE);
        if (var.cmd_airspeed > apcfg.taxi_gSpeed) {
            if (var.delta < var.gSpeed * 10)
                var.cmd_airspeed = apcfg.taxi_gSpeed;
        }
        if (var.delta > (apcfg.taxi_turnR > 0 ? apcfg.taxi_turnR : 3))
            break;
        var.twidx++;
        prev_twidx = var.twidx;
        var.stage--;
    } break;
    default:
        if (var.stage < 100) {
            var.twidx++;
            var.stage = 3;
        } else {
            dmsg("Taxi reset\n");
            var.stage = 0;
        }
        break;
    }
    set_ail(var.rc_roll);
    set_elv(var.rc_pitch, false);
    set_thr(var.rc_throttle);
}
bool Shiva::setup_taxiway()
{
    //check GPS home pos and init mission
    Mission::_item_tw *tw = mission.tw(var.twidx);
    Mission::_item_tw *tw_next = mission.tw(var.twidx + 1);
    if ((!tw) || (!tw_next) || (!(var.status & status_home)))
        return false;
    appNE = var.llh2ne(Vect(tw->lat, tw->lon, var.home_pos[2]));
    rwNE = var.llh2ne(Vect(tw_next->lat, tw_next->lon, var.home_pos[2]));
    rwHDG = var.heading(rwNE - appNE);
    var.mtype = mtype_line;
    var.tgHDG = rwHDG;
    var.cmd_course = var.tgHDG;
    return true;
}
//==============================================================================
void Shiva::TAKEOFF(void)
{
    if (prev_rwidx != var.rwidx) { //protect runway change
        if (var.stage < TAKEOFF_stage::Start)
            var.stage = 0;
        else if (var.stage != TAKEOFF_stage::WaitFP_do) {
            var.rwidx = prev_rwidx;
            dmsg("Can't select runway.\n");
        }
        if (prev_rwidx != var.rwidx) {
            dmsg("Select runway #%u.\n", var.rwidx + 1);
            prev_rwidx = var.rwidx;
        }
    }
    if ((!prev_sgps) && (var.status & status_gps)) { //protect home pos change
        if (var.stage < TAKEOFF_stage::Start)
            var.stage = 0;
        prev_sgps = 1;
    }
    uint32_t stage = var.stage;
    switch (stage) {
    case TAKEOFF_stage::init: {
        stage++;
        if (!apcfg.init_ignition)
            var.power |= power_ignition;
        var.ctrb &= ~ctrb_ers;
        var.ctrb &= ~ctrb_rel;
        var.power &= ~power_agl;
        var.sw |= sw_taxi;
        var.rwAdj = 0;
        set_vspeed(0, false);
        var.rc_throttle = 0;
        reg_Brakes.reset();
        var.ctr_brake = 1;
        //check GPS home pos and init mission
        Mission::_item_rw *rw = mission.rw(var.rwidx);
        if (!rw) {
            var.stage = TAKEOFF_stage::WaitFP;
            return;
        }
        if (!(var.status & status_home)) {
            var.stage = TAKEOFF_stage::WaitPOS;
            return;
        }
        rwNE = var.llh2ne(Vect(rw->lat, rw->lon, rw->hmsl));
        rwHDG = var.heading(Point(rw->dN, rw->dE));
        dmsg("Runway #%u. HDG:%i\n", var.rwidx + 1, (int) var.boundAngle360(rwHDG));
        dmsg("Takeoff: Taxi\n");
    } break;
    case TAKEOFF_stage::Taxi: {
    } break;
    case TAKEOFF_stage::Start: {
        var.home_pos[2] = var.gps_pos[2]; //reset home altitude
        var.altps_gnd = var.altps;        //reset ground
        stage++;
    } break;
    case TAKEOFF_stage::Start_do: {
    } break;
    case TAKEOFF_stage::Run: {
        stage++;
        if (apcfg.to_type == to_type_taxitest) {
            stage = TAKEOFF_stage::Cancel;
            break;
        }
        dmsg("Takeoff: Run\n");
        var.ctr_brake = 0;
    } break;
    case TAKEOFF_stage::Run_do: {
    } break;
    case TAKEOFF_stage::Liftoff: {
        stage++;
        dmsg("Takeoff: Liftoff\n");
    } break;
    case TAKEOFF_stage::Liftoff_do: {
    } break;
    case TAKEOFF_stage::Climbing: {
        stage++;
        dmsg("Takeoff: Climbing\n");
        var.sw &= ~sw_taxi;
        var.power &= ~power_agl;
    } break;
    case TAKEOFF_stage::Climbing_do: {
    } break;
    case TAKEOFF_stage::Finished: {
        dmsg("Takeoff procedure finished\n");
        var.mode = mode_WPT;
    } break;
    //------------------
    case TAKEOFF_stage::WaitFP:
        dmsg("Runway #%u not exist.\n", var.rwidx + 1);
        dmsg("Waiting for flight plan...\n");
        stage++;
        break;
    case TAKEOFF_stage::WaitFP_do:
        if (mission.rw(var.rwidx))
            stage = 0;
        break;
    case TAKEOFF_stage::WaitFP_done:
        stage = TAKEOFF_stage::Cancel; //ignore user stage change
        break;
    case TAKEOFF_stage::WaitPOS:
        dmsg("Waiting for position fix...\n");
        stage++;
        break;
    case TAKEOFF_stage::WaitPOS_do:
        if (var.status & status_gps)
            stage = 0;
        break;
    case TAKEOFF_stage::WaitPOS_done:
        stage = TAKEOFF_stage::Cancel; //ignore user stage change
        break;
    case TAKEOFF_stage::Cancel:
    default:
        dmsg("Takeoff cancelled\n");
        stage = 0;
        break;
    }
    uint32_t stage_s = stage;
    switch (apcfg.type) {
    case type_airplane:
        stage = TAKEOFF_fixedwing(var.stage);
        break;
    case type_helicopter:
        stage = TAKEOFF_helicopter(var.stage);
        break;
    case type_blimp:
        stage = TAKEOFF_blimp(var.stage);
        break;
    case type_boat:
        stage = TAKEOFF_boat(var.stage);
        break;
    }
    if (var.stage == stage)
        var.stage = stage_s;
    else
        var.stage = stage;
}
//==============================================================================
uint32_t Shiva::TAKEOFF_fixedwing(uint32_t stage)
{
    switch (stage) {
    case TAKEOFF_stage::init: {
        var.mtype = mtype_line;
        var.tgHDG = rwHDG;
        if (apcfg.to_type == to_type_launcher)
            var.tgHDG = var.theta[2]; //reset yaw
        var.cmd_course = var.tgHDG;
        setNE(rwNE);
        var.cmd_airspeed = 0;
        if (apcfg.flaps_takeoff > 0) {
            var.ctr_flaps = apcfg.flaps_takeoff;
            doAutoFlaps = false;
        }
    } break;
    case TAKEOFF_stage::Taxi: {
        flytoLine(rwNE);
        set_roll(0, false);
        set_pitch(0);
        /*if(apcfg.to_runPitch==0 || apcfg.to_runPitch>=apcfg.to_liftPitch){
        pitch_s=var.theta[1];
        if(pitch_s<0)pitch_s=0;
      }else */
        pitch_s = apcfg.to_runPitch;
        stabilize();
        reg_Roll.reset();
        ctr_Taxi();
        //taxi manual
        reg_Yaw.reset();
        set_steering(var.rc_roll);
        set_rud(var.rc_roll, false);
        set_thr(var.rc_throttle);
        //check ground speed and brake
        //if(apcfg.to_taxiSpeed!=0)ctr_Brakes();
        //power-on-reset launcher mode
        //if(power_on_reset && apcfg.to_type==to_launcher && apcfg.init_mode==fmi_LAUNCHER)
        //stage++;
    } break;
    case TAKEOFF_stage::Start: {
        var.ctr_brake = 1;
        set_roll(0);
        set_pitch(0);
        if (apcfg.to_type == to_type_taxitest)
            dmsg("Takeoff: Taxi test\n");
        else if (apcfg.to_type == to_type_launcher)
            dmsg("Takeoff: Launcher engaged\n");
        else
            dmsg("Takeoff: Start\n");
    } break;
    case TAKEOFF_stage::Start_do: {
        bool doThrStart = false;
        if (apcfg.to_type == to_type_launcher) {
            var.calc();
            var.tgHDG = var.theta[2]; //reset yaw
            var.cmd_course = var.tgHDG;
            pitch_s = apcfg.to_liftPitch;
            set_roll(0, false);
            set_pitch(pitch_s);
            stabilize();
            reg_Roll.reset();
            set_rud(0, false);
            if (apcfg.to_launcherDelay > 0) {
                reg_Pitch.reset();
                set_ail(0);
                set_elv(0, false);
            }
        } else {
            flytoLine(rwNE);
            set_roll(0, false);
            set_pitch(pitch_s);
            stabilize();
            reg_Roll.reset();
            reg_Pitch.reset();
            ctr_Taxi();
            ctr_Yaw();
        }
        //set_thr(var.ctr_throttle);
        if (apcfg.to_type == to_type_taxitest) {
            set_thr(var.rc_throttle); //override, no mix
        } else if (apcfg.to_type == to_type_launcher) {
            if (apcfg.to_thrDelay < 0)
                doThrStart = true;
            if (var.acc[0] >= ((apcfg.to_launcherAx == 0) ? (9.81 * 1.5) : apcfg.to_launcherAx))
                stage++;
        } else {
            doThrStart = true;
            if (var.ctr_throttle >= 1)
                stage++;
        }
        if (doThrStart) {
            //increase throttle slowly
            if (var.ctr_throttle < 1) {
                if (apcfg.to_thrRise > 0)
                    set_thr(var.ctr_throttle + apcfg.to_thrRise * dt);
                else
                    set_thr(1);
            }
        }
    } break;
    case TAKEOFF_stage::Run: {
        if (apcfg.to_type == to_type_launcher) {
            if (apcfg.to_launcherDelay > 0) {
                time_s = t;
            } else
                stage = TAKEOFF_stage::Liftoff;
        } else if (apcfg.to_liftSpeed == 0) {
            stage = TAKEOFF_stage::Liftoff;
        }
        var.cmd_airspeed = apcfg.to_liftSpeed;
    } break;
    case TAKEOFF_stage::Run_do: {
        if (apcfg.to_type == to_type_launcher) { //enters only if to_launcherDelay>0
            var.tgHDG = var.course;              //reset yaw
            var.cmd_course = var.tgHDG;
            set_roll(0, false);
            set_pitch(pitch_s);
            stabilize();
            reg_Roll.reset();
            reg_Pitch.reset();
            set_ail(0);
            set_elv(0, false);
            if ((t - time_s) >= (apcfg.to_launcherDelay * 1000))
                stage++;
        } else {
            flytoLine(rwNE);
            set_roll(0, false);
            set_pitch(pitch_s);
            stabilize();
            reg_Roll.reset();
            reg_Pitch.reset();
            ctr_Taxi();
            ctr_Yaw();
            if (var.airspeed > apcfg.to_liftSpeed)
                stage++;
        }
        if (apcfg.to_thrDelay <= 0)
            set_thr(1);
    } break;
    case TAKEOFF_stage::Liftoff: {
        var.cmd_airspeed = (apcfg.spd_VY == 0) ? apcfg.spd_cruise : apcfg.spd_VY;
        reg_Course.reset();
        pitch_s = apcfg.to_liftPitch * 0.3; // minimum for liftoff pitch
        if (apcfg.to_liftAGL)
            var.power |= power_agl;
        else
            var.power &= ~power_agl;
        time_s = time_thr_s = t;
        if (apcfg.flaps_takeoff > var.ctr_flaps)
            var.ctr_flaps = apcfg.flaps_takeoff;
    } break;
    case TAKEOFF_stage::Liftoff_do: {
        if (apcfg.to_type == to_type_launcher) {
            var.calc();
            var.tgHDG = var.course; //reset yaw
            var.cmd_course = var.tgHDG;
        } else {
            flytoLine(rwNE);
        }
        ctr_Course();
        //limit roll
        if (apcfg.to_type == to_type_launcher) {
            var.cmd_theta[0] = 0;
            reg_Course.reset();
        } else {
            var.cmd_theta[0] = var.limit(var.cmd_theta[0], var.altitude < 0 ? 0 : var.altitude);
        }
        //pitch down with altitude
        _var_float v = (1 - var.altitude / (apcfg.to_liftAlt)) * (apcfg.to_liftPitch);
        v = var.limit(v, pitch_s, apcfg.to_liftPitch);
        set_pitch(v, false);
        reg_Airspeed.reset(var.cmd_theta[1]);
        stabilize();
        if (apcfg.flaps_takeoff <= 0)
            ctr_autoflaps();
        //check if done
        if (!(((var.altitude > (apcfg.to_liftAlt * 0.5)) && (var.airspeed > var.cmd_airspeed))
              || (var.altitude > apcfg.altitude)))
            time_s = t;
        else if ((t - time_s) > 200)
            stage++;
        if (apcfg.to_type == to_type_launcher) {
            //increase throttle slowly and delay
            if (apcfg.to_thrDelay > 0 && (t - time_thr_s) >= (apcfg.to_thrDelay * 1000)) {
                if (var.ctr_throttle < 1) {
                    if (apcfg.to_thrRise > 0)
                        set_thr(var.ctr_throttle + apcfg.to_thrRise * dt);
                    else
                        set_thr(1);
                }
            }
            set_thr(var.ctr_throttle);
        } else {
            ctr_Taxi();
            ctr_Yaw();
            set_thr(1);
        }
    } break;
    case TAKEOFF_stage::Climbing: {
        set_steering(0);
    } break;
    case TAKEOFF_stage::Climbing_do: {
        if (apcfg.to_type == to_type_launcher)
            var.calc();
        else
            flytoLine(rwNE);
        var.cmd_course = var.tgHDG;
        navigate();
        set_thr((apcfg.to_climbThr <= 0) ? 1 : apcfg.to_climbThr);
        if (var.altitude > (var.cmd_altitude * 0.8)) {
            stage++;
            if (apcfg.flaps_takeoff > 0 && apcfg.flaps_auto == 0)
                var.ctr_flaps = 0;
        }
        calcETA((var.cmd_altitude * 0.8 - var.altitude), -var.gps_vel[2], false);
    } break;
    }
    return stage;
}
//==============================================================================
uint32_t Shiva::TAKEOFF_helicopter(uint32_t stage)
{
    switch (stage) {
    case TAKEOFF_stage::init: {
        var.cmode |= cmode_hover;
        setNE(var.pos_NE);
    } break;
    case TAKEOFF_stage::Taxi: {
        var.calc();
        set_roll(0, false);  //set by rc adjust
        set_pitch(0, false); //set by rc adjust
        set_yaw(var.theta[2], false);
        var.cmd_course = var.theta[2]; //reset yaw
        reg_Yaw.reset();
        stabilize();
        var.ctr_collective = var.rc_throttle * 2.0 - 1.0;
        reg_VSpeed.reset(var.ctr_collective * 100.0);
        var.cmd_altitude = var.altitude;
        set_thr(var.rc_throttle); //override, no mix
        var.cmode &= ~cmode_hyaw;
    } break;
    case TAKEOFF_stage::Start: {
        var.ctr_brake = 0;
        setNE(var.pos_NE);
        var.rc_roll = 0;
        var.rc_pitch = 0;
        dmsg("Takeoff: Start\n");
    } break;
    case TAKEOFF_stage::Start_do: {
        var.calc();
        set_roll(0, false);  //set by rc adjust
        set_pitch(0, false); //set by rc adjust
        reg_Roll.reset();
        reg_Pitch.reset();
        set_yaw(var.theta[2], false);
        var.cmd_course = var.theta[2]; //reset yaw
        reg_Yaw.reset();
        stabilize();
        var.ctr_collective = apcfg.to_liftPitch;
        reg_VSpeed.reset(var.ctr_collective * 100.0);
        var.cmd_altitude = var.altitude;
        //increase throttle slowly
        if (var.ctr_throttle < 1.0)
            set_thr(var.ctr_throttle + apcfg.to_thrRise * dt);
        var.cmd_rpm = apcfg.rpm;
        if (var.rpm >= apcfg.rpm && apcfg.to_type == to_type_runway)
            stage++;
    } break;
    case TAKEOFF_stage::Run: {
        time_s = t;
        setNE(var.pos_NE);
    } break;
    case TAKEOFF_stage::Run_do: {
        if ((t - time_s) >= 1000)
            stage++;
        var.calc();
        set_roll(0, false);  //set by rc adjust
        set_pitch(0, false); //set by rc adjust
        reg_Roll.reset();
        reg_Pitch.reset();
        set_yaw(var.theta[2], false);
        var.cmd_course = var.theta[2]; //reset yaw
        reg_Yaw.reset();
        //set_yaw(var.cmd_course,false);
        stabilize();
        var.ctr_collective = apcfg.to_liftPitch;
        reg_VSpeed.reset(var.ctr_collective * 100.0);
        var.cmd_altitude = apcfg.altitude;
        var.cmd_rpm = apcfg.rpm;
        ctr_Governor();
    } break;
    case TAKEOFF_stage::Liftoff: {
        setNE(var.pos_NE);
    } break;
    case TAKEOFF_stage::Liftoff_do: {
        var.calc();
        navigate();
        reg_HoverX.reset();
        reg_HoverY.reset();
        //boost pitch grow
        if (apcfg.heli_Kto > 1 && var.vspeed < var.cmd_vspeed) {
            reg_VSpeed.sum = var.limit(reg_VSpeed.sum + apcfg.heli_Kto * dt, apcfg.VSpeed.Li);
        }
        if (var.altitude >= apcfg.to_liftAlt)
            stage++;
    } break;
    case TAKEOFF_stage::Climbing: {
        if (conf.ahrs_noMAG != ahrs_noMAG_always)
            var.cmode &= ~cmode_nomag;
    } break;
    case TAKEOFF_stage::Climbing_do: {
        var.calc();
        navigate();
        if (var.altitude >= apcfg.altitude * 0.8)
            stage++;
    } break;
    }
    return stage;
}
//==============================================================================
uint32_t Shiva::TAKEOFF_blimp(uint32_t stage)
{
    switch (stage) {
    case TAKEOFF_stage::init: {
    } break;
    case TAKEOFF_stage::Taxi: {
    } break;
    case TAKEOFF_stage::Start: {
    } break;
    case TAKEOFF_stage::Start_do: {
    } break;
    case TAKEOFF_stage::Run: {
    } break;
    case TAKEOFF_stage::Run_do: {
    } break;
    case TAKEOFF_stage::Liftoff: {
    } break;
    case TAKEOFF_stage::Liftoff_do: {
    } break;
    case TAKEOFF_stage::Climbing: {
    } break;
    case TAKEOFF_stage::Climbing_do: {
    } break;
    }
    return stage;
}
//==============================================================================
uint32_t Shiva::TAKEOFF_boat(uint32_t stage)
{
    switch (stage) {
    case TAKEOFF_stage::init: {
    } break;
    case TAKEOFF_stage::Taxi: {
    } break;
    case TAKEOFF_stage::Start: {
    } break;
    case TAKEOFF_stage::Start_do: {
    } break;
    case TAKEOFF_stage::Run: {
    } break;
    case TAKEOFF_stage::Run_do: {
    } break;
    case TAKEOFF_stage::Liftoff: {
    } break;
    case TAKEOFF_stage::Liftoff_do: {
    } break;
    case TAKEOFF_stage::Climbing: {
    } break;
    case TAKEOFF_stage::Climbing_do: {
    } break;
    }
    return stage;
}
//==============================================================================
//==============================================================================
void Shiva::LANDING(void)
{
    if (prev_rwidx != var.rwidx) { //protect runway change
        prev_rwidx = var.rwidx;
        if (var.stage)
            var.stage = LANDING_stage::Cancel;
    }
    uint32_t stage = var.stage;
    switch (stage) {
    case LANDING_stage::init: {
        stage++;
        var.status &= ~status_touch;
        var.ctr_airbrk = 0;
        var.cmd_airspeed = apcfg.spd_cruise;
        if (apcfg.ld_AGL == ld_AGL_always)
            var.power |= power_agl;
        else
            var.power &= ~power_agl;
        var.rwAdj = 0;
        var.mtype = mtype_hdg;
        alt_corr = 0;
        ers_landing = apcfg.ld_type == ld_type_parachute;
        if (var.ctrb & ctrb_ers) {
            ers_landing = true;
            stage = LANDING_stage::Flare;
            break;
        }
        //runway
        Mission::_item_rw *rw = mission.rw(var.rwidx);
        if (!rw) {
            dmsg("Runway #%u not exist.\n", var.rwidx + 1);
            if (ers_landing) {
                stage = LANDING_stage::Flare;
                break;
            } else {
                var.mode = mode_HOME;
                return;
            }
        }
        rwNE = var.llh2ne(Vect(rw->lat, rw->lon, rw->hmsl));
        rwHDG = var.heading(Point(rw->dN, rw->dE));
        rwLeft = rw->hdr.option == Mission::mo_left;
        appLength = rw->approach;
        if (rw->hmsl != 0 && var.home_pos[2] != rw->hmsl) {
            var.home_pos[2] = rw->hmsl;
            dmsg("Home hmsl adjusted: %im\n", (int) var.home_pos[2]);
        }
        dmsg("Runway #%u. HDG:%i\n", var.rwidx + 1, (int) var.boundAngle360(rwHDG));
    } break;
    case LANDING_stage::TA: {
        dmsg("Landing: TA\n");
        stage++;
    } break;
    case LANDING_stage::TA_do: {
    } break;
    case LANDING_stage::TAapp: {
        dmsg("Landing: TA_app\n");
        var.ctr_brake = 0;
        stage++;
    } break;
    case LANDING_stage::TAapp_do: {
    } break;
    case LANDING_stage::App: {
        dmsg("Landing: Final Approach\n");
        var.status &= ~status_touch;
        stage++;
    } break;
    case LANDING_stage::App_do: {
    } break;
    case LANDING_stage::Flare: {
        dmsg("Landing: Flare\n");
        alt_corr = 0;
        alt_speech = (int) var.altitude;
        if (alt_speech > 10)
            alt_speech = 10;
        time_s = t;
        if (apcfg.ld_AGL)
            var.power |= power_agl;
        var.status &= ~status_touch;
        stage++;
    } break;
    case LANDING_stage::Flare_do: {
        if (ers_landing)
            break;
        //say AGL altitude
        if ((var.altitude >= 0) && (var.altitude < alt_speech)) {
            dmsg("Landing: %u m to go\n", alt_speech);
            if (alt_speech >= 10)
                alt_speech = 7;
            else if (alt_speech >= 7)
                alt_speech = 5;
            else if (alt_speech >= 5)
                alt_speech = 3;
            else
                alt_speech--;
        }
        //check if touchdown
        if ((apcfg.ld_tdEvent == ld_tdEvent_altitude
             || (apcfg.ld_tdEvent == ld_tdEvent_agl && (!apcfg.ld_AGL)))
            || ((apcfg.ld_tdEvent == ld_tdEvent_agl) && (var.status & status_agl))) {
            if (var.altitude > apcfg.ld_tdAlt)
                time_s = t;
            else if ((t - time_s) > 200)
                stage = LANDING_stage::Touchdown;
        }
    } break;
    case LANDING_stage::Flare_done: {
        alt_corr = apcfg.ld_tdAlt - var.altitude;
        stage++;
    } break;
    case LANDING_stage::Touchdown: {
        dmsg("Landing: Touchdown\n");
        time_s = t;
        stage++;
    } break;
    case LANDING_stage::Touchdown_do: {
    } break;
    case LANDING_stage::Run: {
        var.status |= status_touch;
        dmsg("Landing: Run\n");
        stage++;
    } break;
    case LANDING_stage::Run_do: {
    } break;
    case LANDING_stage::Run_done: {
        stage = LANDING_stage::Finished;
    } break;
    case LANDING_stage::Cancel: {
        dmsg("Landing cancelled\n");
        stage++;
        var.ctrb &= ~ctrb_ers;
        var.ctrb &= ~ctrb_rel;
        var.status &= ~status_touch;
    } break;
    case LANDING_stage::Cancel_do: {
    } break;
    case LANDING_stage::Cancel_done: {
        stage = 0;
    } break;
    case LANDING_stage::Finished: {
        dmsg("Landing finished\n");
        stage++;
    } break;
    case LANDING_stage::Finished_do: {
    } break;
    case LANDING_stage::Finished_done: {
        stage = LANDING_stage::Finished;
    } break;
    }
    uint32_t stage_s = stage;
    switch (apcfg.type) {
    case type_airplane:
        stage = LANDING_fixedwing(var.stage);
        break;
    case type_helicopter:
        stage = LANDING_helicopter(var.stage);
        break;
    case type_blimp:
        stage = LANDING_blimp(var.stage);
        break;
    case type_boat:
        stage = LANDING_boat(var.stage);
        break;
    }
    if (var.stage == stage)
        var.stage = stage_s;
    else
        var.stage = stage;
}
//==============================================================================
uint32_t Shiva::LANDING_fixedwing(uint32_t stage)
{
    switch (stage) {
    case LANDING_stage::init: {
        var.ctr_airbrk = 0;
        reg_AirbrkApp.reset();
        reg_AirbrkDS.reset();
        var.delta = 0;
        doAutoFlaps = true;
    } break;
    case LANDING_stage::TA: {
        var.delta = 0;
        cR = apcfg.turnR * 2.0;
        aLen = appLength;
        var.turnR = rwLeft ? -cR : cR;
        var.delta = 0;
        get_landing_length(ers_landing ? apcfg.ld_flareAlt : 0);
        //set altitude
        _var_float v;
        if (apcfg.ldratioMin > 0)
            v = apcfg.ldratioMin;
        else if (var.ldratio > 0)
            v = var.ldratio;
        else
            v = 10;
        v = (int) (2.0 * (apcfg.turnR * (2.0 * PI) + aLen) / v / app_Kwt) / 10 * 10;
        if (ers_landing)
            v += apcfg.ld_flareAlt;
        v = var.limit(v, apcfg.altitude, apcfg.altitude * 3);
        if (var.cmd_altitude < v)
            var.cmd_altitude = v;
        //set airspeed
        if (apcfg.spd_VNO > apcfg.spd_cruise) {
            v = (int) (var.windSpd * 1.5);
            v = var.limit(v, apcfg.spd_cruise, apcfg.spd_VNO);
            if (var.cmd_airspeed < v)
                var.cmd_airspeed = v;
        }
        dmsg("Landing: altitude %im\n", (int) var.cmd_altitude);
        if (ers_landing)
            dmsg("Landing: Parachute\n");
        sub_stage = 0;
    } break;
    case LANDING_stage::TA_do: {
        //fly closer to approach circle...
        get_landing_length(ers_landing ? apcfg.ld_flareAlt : 0);
        //check if on circle side
        Point ne = rwNE - var.pos_NE;
        _var_float rwDelta = var.distance(ne) * std::sin((var.heading(ne) + 180.0 - rwHDG) * D2R);
        bool bSideOk = rwLeft ? (rwDelta < 0) : (rwDelta > 0);
        //var.user2=rwDelta;
        if (var.delta > (vLE_est * 0.5) || (!bSideOk)) { //loiter to descend
            if (!sub_stage) {
                sub_stage++;
                var.turnR = apcfg.turnR * 2.0;
                if (rwLeft)
                    var.turnR = -var.turnR;
                const _var_float vf = apcfg.ld_flareAlt * 1.5;
                const _var_float vs = apcfg.altitude * 1.5;
                var.cmd_altitude = vf > vs ? vf : vs; //max
            }
        } else if (var.delta > 0 && (var.dWPT - cR) < (cR))
            stage++; //cSpan>200 && std::abs(var.dWPT-cR)<(cR) &&
        flytoCircle(var.cmd_NE, std::abs(var.turnR), rwLeft, false);
        navigate();
        reg_AirbrkApp.reset((var.ctr_airbrk - apcfg.ld_Airbrk_bias) * 100.0);
        reg_AirbrkDS.reset((var.ctr_airbrk - apcfg.ld_Airbrk_bias) * 100.0);
        calcETA(vLE_total, var.airspeed);
    } break;
    case LANDING_stage::TAapp: {
        time_s = time_stage_s = t;
        var.cmd_altitude = ers_landing ? apcfg.ld_flareAlt : 0;
        if (apcfg.spd_VNO >= apcfg.spd_cruise)
            var.cmd_airspeed = apcfg.spd_VNO;
        if (apcfg.flaps_landing > 0) {
            doAutoFlaps = false;
            var.ctr_flaps = apcfg.flaps_landing;
        }
        dmsg("Landing: path %.1fkm, est %.1fkm\n", vLE_total / 1000.0, vLE_est / 1000.0);
    } break;
    case LANDING_stage::TAapp_do: {
        //fly by approach circle
        if ((t - time_stage_s) < 3000)
            var.delta = 0; //kf reset
        if (!adjust_landing_circle()) {
            stage++;
        }
        if (apcfg.ld_autocancel && (!(var.error & error_rpm)) && (!(var.cmode & cmode_thrcut))
            && (!(var.cmode & cmode_throvr))) {
            if (var.delta > (-appLength))
                time_s = t;
            else if ((t - time_s) > 5000)
                stage = LANDING_stage::Cancel; //cancel
        }
        flytoCircle(var.cmd_NE, cR, rwLeft, false);
        //adjust speed (ldratio)
        if (apcfg.spd_VNO > apcfg.spd_cruise) {
            _var_float v;
            v = var.limit((int) (var.windSpd * 1.5), apcfg.spd_cruise, apcfg.spd_VNO); //low limit
            v = var.limit(apcfg.spd_cruise + (apcfg.spd_VNO - apcfg.spd_cruise) / 2.0
                              + var.delta * apcfg.ld_speedKp,
                          v,
                          apcfg.spd_VNO);
            var.cmd_airspeed = v;
        }
        navigate();
        set_thr(apcfg.ld_thr);
        ctr_AirbrkApp();
        calcETA(vLE_total, var.airspeed);
    } break;
    case LANDING_stage::App: {
        var.tgHDG = rwHDG;
        time_s = t;
    } break;
    case LANDING_stage::App_do: {
        flytoLine(rwNE);
        //calc delta
        vLE_total = (var.altitude - var.cmd_altitude) * var.ldratio;
        _var_float dWP = var.dWPT;
        if (std::abs(var.boundAngle(var.wpHDG - rwHDG)) > 90)
            dWP = -dWP;
        vLE_est = dWP / var.wind_triangle(rwHDG);
        setDelta(vLE_total - vLE_est);
        if (apcfg.ld_autocancel && (!(var.error & error_rpm)) && (!(var.cmode & cmode_thrcut))
            && (!(var.cmode & cmode_throvr))) {
            bool bOk = true;
            if (dWP > 0) {           //approaching
                if (var.delta < 0) { //undershoot checks only
                    bOk = dWP > (appLength * 0.5)
                          || var.delta
                                 > (var.cmd_airspeed * -7.0); //check delta on second half only
                    bOk &= dWP > (appLength * 0.3)
                           || std::abs(var.rwDelta + var.rwAdj) < 20; //check alignment
                    bOk &= ers_landing
                           || var.altitude
                                  > apcfg.ld_flareAlt; // || var.delta>(var.cmd_airspeed*-7.0);
                }
            } else { //overshooted RW startpoint
                _var_float dMax = apcfg.ld_maxRun;
                if (dMax <= 0)
                    dMax = aLen;
                bOk = var.delta < dMax;
            }
            if (bOk)
                time_s = t;
            else if ((t - time_s) > 5000)
                stage = LANDING_stage::Cancel; //cancel
        }
        //adjust speed (ldratio)
        if (apcfg.spd_VNO > apcfg.spd_cruise) {
            //LD control
            _var_float v = apcfg.spd_cruise + (apcfg.spd_VNO - apcfg.spd_cruise) / 2.0
                           + var.delta * apcfg.ld_speedKp;
            //dist vs wind control
            if (app_Kwt < 1.0)
                v -= var.delta / (app_Kwt) *apcfg.ld_windKp;
            var.cmd_airspeed = var.limit(v, apcfg.spd_cruise, apcfg.spd_VNO);
        }
        if (ers_landing) {
            //limit airspeed proportionally to distance
            /*_var_float v=apcfg.spd_VNO>apcfg.spd_cruise?apcfg.spd_VNO:apcfg.spd_cruise;
        if(appLength>0 && apcfg.ld_runSpeed<v){
          v-=apcfg.ld_runSpeed;
          v=apcfg.ld_runSpeed+v*dWP/appLength;
          if(var.cmd_airspeed>v)var.cmd_airspeed=v;
        }*/
            if (dWP < 0)
                stage++;
        } else if (var.altitude <= apcfg.ld_flareAlt)
            stage++;
        navigate();
        set_thr(apcfg.ld_thr);
        ctr_AirbrkApp();
        calcETA(vLE_total, var.airspeed);
    } break;
    case LANDING_stage::Flare: {
        speed_s = var.cmd_airspeed; //save for glider recovery
        pitch_s = pitch_s2 = pitch_s3
            = var.cmd_theta[1]; //to only increase pitch if below ld_finMinPitch
        reg_VSpeed.reset(pitch_s);
        if (ers_landing) {
            time_s = t;
            var.power &= ~power_ignition;
            var.cmd_airspeed = apcfg.ld_runSpeed;
            //var.ctrb|=ctrb_ers;
        }
        var.ETA = 0;
    } break;
    case LANDING_stage::Flare_do: {
        flytoLine(rwNE);
        if ((!ctr_AirbrkDS(apcfg.ld_flareDS)) && var.ctr_airbrk > apcfg.ld_Airbrk_bias) {
            var.ctr_airbrk -= dt / 3.0;
            if (var.ctr_airbrk < apcfg.ld_Airbrk_bias)
                var.ctr_airbrk = apcfg.ld_Airbrk_bias;
        }
        ctr_Course();
        _var_float delta = 0;
        //-----------------------------
        if (ers_landing) {
            if (!(var.ctrb & ctrb_ers)) {
                if (((t - time_s) > 3000 && var.vspeed < 0) || var.airspeed < apcfg.ld_runSpeed) {
                    //stage++;
                    dmsg("Landing: Descending\n");
                    var.ctrb |= ctrb_ers;
                    time_s = t;
                }
                ctr_Airspeed();
                if (apcfg.ld_flarePitch > 0)
                    set_pitch(apcfg.ld_flarePitch);
            } else { //wait until stabilize
                if ( //(apcfg.ld_runGy==0||var.gyro.mag()<apcfg.ld_runGy) &&
                    (apcfg.ld_flareDS == 0 || std::abs(var.vspeed) < apcfg.ld_flareDS)
                    && (apcfg.ld_runAz == 0 || var.acc.mag() < (9.81 + apcfg.ld_runAz))) {
                    if ((t - time_s) > 3000) {
                        stage++;
                        time_s = t;
                    }
                } else
                    time_s = t;
            }
            //-----------------------------
        } else if (apcfg.ld_flareDS > 0 && apcfg.VSpeed.Lo > 0) {
            /*//decrease vspeed with altitude
        _var_float v=0;//apcfg.ld_tdAlt;
        v=((var.altitude-v)/(apcfg.ld_flareAlt-v));
        v=var.limit(v*apcfg.ld_flareDS,0,apcfg.ld_flareDS);
        set_vspeed(-v,true);*/
            _var_float v = 0;
            if (var.cmd_theta[1] >= apcfg.ld_flarePitch)
                v = -apcfg.ld_flareDS;
            set_vspeed(v, true);
            ctr_VSpeed();
            //pitch down limit, gradient pitch down protect
            if (var.cmd_theta[1] < apcfg.ld_flarePitch) {
                if (var.cmd_theta[1] < pitch_s)
                    set_pitch(pitch_s, false);
                else
                    pitch_s = var.cmd_theta[1];
            }
        } else {
            ctr_Altitude();
            //decrease airspeed with altitude
            _var_float v = apcfg.ld_tdAlt;
            v = ((var.altitude - v) / (apcfg.ld_flareAlt - v));
            v = apcfg.spd_stall + v * (speed_s - apcfg.spd_stall); //smooth from saved cmd_airspeed
            if (v < apcfg.spd_stall)
                v = apcfg.spd_stall;
            if (var.cmd_airspeed > v)
                var.cmd_airspeed = v; //only decrease
            ctr_Airspeed();
            //pitch up limit by altitude
            if (apcfg.ld_flarePitch > 0) { // && pitch_s2<apcfg.ld_flarePitch){
                v = 0;                     //apcfg.ld_tdAlt;
                v = ((var.altitude - v) / (apcfg.ld_flareAlt - v));
                _var_float vspan = apcfg.ld_tdPitch + apcfg.ld_flarePitch;
                v = apcfg.ld_tdPitch - vspan * v; //-(apcfg.ld_flarePitch*2.0)*v;
                if (v < pitch_s3)
                    v = pitch_s3;
                else
                    pitch_s3 = v; //growing limit only
                if (reg_Airspeed.out > v) {
                    set_pitch(v, false);
                    reg_Airspeed.reset(v);
                }
            }
            //pitch down limit, gradient pitch down protect
            if (reg_Airspeed.out < 0 || var.altitude < 10) {
                if (reg_Airspeed.out < pitch_s)
                    set_pitch(pitch_s, false);
                else
                    pitch_s = reg_Airspeed.out;
            } else
                pitch_s = reg_Airspeed.out;
        }

        //calc delta
        vLE_total = var.altitude * var.ldratio;
        delta = var.dWPT;
        if (std::abs(var.boundAngle(var.wpHDG - rwHDG)) > 90)
            delta = -delta;
        setDelta(vLE_total - delta / var.wind_triangle(rwHDG));

        stabilize();
        ctr_autoflaps();
        set_thr(ers_landing ? 0 : apcfg.ld_thr);
        if (apcfg.ld_tdEvent == ld_tdEvent_hook) {
            if (var.gSpeed < apcfg.ld_runSpeed) {
                stage++; //hanging
                break;
            } else if (apcfg.ld_autocancel && (!(var.error & error_rpm))) {
                //autocancel
                if (delta > var.gSpeed) { //1sec miss
                    stage = LANDING_stage::Cancel;
                    break;
                }
            }
        }
    } break;
    case LANDING_stage::Flare_done: {
    } break;
    case LANDING_stage::Touchdown: {
        if (apcfg.ld_tdEvent == ld_tdEvent_hook) {
            stage = LANDING_stage::Finished;
            break;
        }
        pitch_s = pitch_s2 = var.cmd_theta[1];
        if (pitch_s < 0)
            pitch_s = 0; //saved cmd_pitch
        var.cmd_airspeed = ers_landing ? apcfg.spd_stall : 0;
        if (apcfg.ld_tdEngine == ld_tdEngine_ignOff) {
            var.cmode |= cmode_thrcut;
            var.power &= ~power_ignition;
        } else if (apcfg.ld_tdEngine == ld_tdEngine_thrCut) {
            var.cmode |= cmode_thrcut;
            var.cmode |= cmode_throvr;
        }
    } break;
    case LANDING_stage::Touchdown_do: {
        if (var.status & status_agl)
            alt_corr = 0;
        flytoLine(rwNE);
        ctr_AirbrkDS(apcfg.ld_flareDS);
        ctr_Course();
        if (ers_landing) {
            set_roll(0);
            ctr_Airspeed();
            if (std::abs(var.vspeed) < 3) {
                if (apcfg.ld_runAz >= 0 && var.acc.mag() >= (9.81 + apcfg.ld_runAz)
                    && (t - time_s) > 200)
                    stage++;
            } else
                time_s = t;
        } else if (apcfg.ld_flareDS > 0 && apcfg.VSpeed.Lo > 0) {
            //SLAVA LANDING ALGORITHM
            //decrease vspeed with altitude
            _var_float v = (var.altitude + alt_corr) / (var.airspeed - apcfg.spd_stall);
            if (var.vcas >= 0 || var.cmd_theta[1] < apcfg.ld_flarePitch)
                v = 0;
            else if (var.vcas < 0)
                v = var.limit(-var.vcas * v, 0.0 * apcfg.ld_flareDS, apcfg.ld_flareDS);
            if (var.airspeed <= apcfg.spd_stall)
                v = apcfg.ld_flareDS;
            set_vspeed(-v, true);
            ctr_VSpeed();
            //pitch down limit, gradient pitch down protect
            if (var.cmd_theta[1] < apcfg.ld_flarePitch) {
                if (var.cmd_theta[1] < pitch_s)
                    set_pitch(pitch_s, false);
                else
                    pitch_s = var.cmd_theta[1];
            }
        } else {
            //pitch up with altitude, limit roll
            _var_float v = var.altitude + alt_corr;
            if (apcfg.Taxi.Lo > 0) {
                //limit roll only when steering available (flying wing option)
                var.cmd_theta[0] = var.limit(var.cmd_theta[0], (v < 2) ? 4 : (v * v));
            }
            v = 1 - (v) / apcfg.ld_tdAlt;
            v = var.limit(v * apcfg.ld_tdPitch, pitch_s, apcfg.ld_tdPitch);
            pitch_s = v; //only increase pitch
            set_pitch(v, false);
            //check stage done
            if (var.airspeed > apcfg.ld_runSpeed) {
                if (apcfg.ld_runAz > 0 && var.acc[2] <= -(9.81 + apcfg.ld_runAz)) {
                    //if(apcfg.ld_runGy<5)stage++;
                    //else if(std::abs(var.gyro[1])>=apcfg.ld_runGy)stage++;
                } else
                    time_s = t;
            }
            if ((t - time_s) > 100)
                stage++;
        }
        stabilize();
        set_thr(0);
        var.theta[2] = var.course;
        var.cmd_theta[2] = var.cmd_course;
        ctr_Yaw();
    } break;
    case LANDING_stage::Run: {
        time_s = t;
        var.ctr_flaps = 0;
        var.ctr_airbrk = 1;
    } break;
    case LANDING_stage::Run_do: {
        flytoLine(rwNE);
        set_roll(0);
        set_pitch(-apcfg.ld_tdPitch * 0.5, true, var.cmd_theta[1] < 0 ? 2 : 4);
        stabilize();
        ctr_Taxi();
        ctr_Yaw();
        set_thr(0);
        //brakes
        if (((t - time_s) > 3000) || (var.gSpeed <= (apcfg.ld_runSpeed * 0.9))) {
            if (var.ctr_brake < 1.0) {
                var.ctr_brake += (apcfg.ld_brkSpeed > 0) ? (apcfg.ld_brkSpeed * dt) : 1;
                if (var.ctr_brake > 1.0)
                    var.ctr_brake = 1.0;
            }
        }
        //finished?
        if (var.gSpeed < (apcfg.ld_runSpeed * 0.3))
            stage++;
    } break;
    case LANDING_stage::Run_done: {
        var.ctr_airbrk = 0;
    } break;
    case LANDING_stage::Cancel: {
        pitch_s = var.cmd_theta[1];
        var.ctr_airbrk = 0;
        reg_AirbrkApp.reset();
        reg_AirbrkDS.reset();
        var.ctr_brake = 0;
        var.ctrb &= ~ctrb_rel;
        set_vspeed(0, false);
        var.cmd_airspeed = (apcfg.spd_VY == 0) ? apcfg.spd_cruise : apcfg.spd_VY;
        var.cmd_altitude = apcfg.altitude;
        doAutoFlaps = true;
        //reg_Airspeed.reset(var.cmd_theta[1]);
    } break;
    case LANDING_stage::Cancel_do: {
        flytoLine(var.cmd_NE);
        var.cmd_course = var.tgHDG;
        navigate(false);
        if ((!ers_landing) && var.cmd_theta[1] < pitch_s)
            set_pitch(pitch_s, false);
        stabilize();
        set_thr((apcfg.to_climbThr <= 0) ? 1 : apcfg.to_climbThr);
        if (var.altitude > (apcfg.altitude * 0.8))
            stage++;
    } break;
    case LANDING_stage::Cancel_done: {
    } break;
    case LANDING_stage::Finished: {
        var.ctr_brake = 1;
        var.power &= ~power_agl;
        if (ers_landing)
            var.ctrb |= ctrb_rel;
    } break;
    case LANDING_stage::Finished_do: {
        var.calc();
        ctr_Taxi();
        ctr_Yaw();
        set_ail(var.rc_roll);
        set_elv(var.rc_pitch, false);
        set_rud(var.rc_yaw, false);
        set_thr(0);
    } break;
    case LANDING_stage::Finished_done: {
    } break;
    }
    return stage;
}
//==============================================================================
uint32_t Shiva::LANDING_helicopter(uint32_t stage)
{
    switch (stage) {
    case LANDING_stage::init: {
        stage = LANDING_stage::TAapp;
        var.tgHDG = rwHDG;
        appNE = rwNE + var.rotate(Point(appLength, 0), -(180.0 + rwHDG));
        setNE(appNE);
        if (var.dWPT > apcfg.wpt_snap && var.altitude > apcfg.ld_flareAlt)
            var.cmode &= ~cmode_hover;
        else
            var.cmode |= cmode_hover;
    } break;
    case LANDING_stage::TA: {
    } break;
    case LANDING_stage::TA_do: {
    } break;
    case LANDING_stage::TAapp: {
        //var.cmd_course=var.tgHDG;
    } break;
    case LANDING_stage::TAapp_do: {
        flytoHover(var.cmd_NE, apcfg.wpt_snap);
        navigate();
        //check if hover stabilized
        if (var.cmode & cmode_hover && var.dWPT <= apcfg.ld_maxRun && var.gSpeed < 1
            && std::abs(var.boundAngle(var.theta[2] - var.cmd_theta[2])) < 30)
            stage++;
    } break;
    case LANDING_stage::App: {
        var.cmd_airspeed = 0;
        var.cmode |= cmode_hover;
        appNE = var.cmd_NE;
        setNE(appNE);
        //var.cmd_course=rwHDG;
        if (appLength <= 0)
            var.cmd_altitude = apcfg.ld_flareAlt * 0.5;
        pitch_s = var.cmd_altitude;
        if (apcfg.ld_AGL)
            var.power |= power_agl;
    } break;
    case LANDING_stage::App_do: {
        setNE(rwNE
              + var.rotate(Point(appLength * var.limit(var.altitude / apcfg.altitude, 0, 1), 0),
                           -(180.0 + rwHDG)));
        ctr_HoverX();
        ctr_HoverY();
        set_yaw(var.cmd_course, false); //user input only
        if (appLength > 0) {
            var.cmd_altitude = apcfg.altitude
                                   * var.limit(var.distance(rwNE - var.pos_NE) / appLength, 0, 1)
                               - 1.0;
        }
        if (var.cmd_altitude > pitch_s)
            var.cmd_altitude = pitch_s;
        else
            pitch_s = var.cmd_altitude;
        ctr_Altitude();
        if (var.cmd_vspeed > -apcfg.ld_flareDS && var.dWPT <= apcfg.ld_maxRun && var.gSpeed < 1)
            set_vspeed(-apcfg.ld_flareDS, false);
        else if (var.cmd_vspeed > 0)
            set_vspeed(0, false);
        ctr_VSpeed();
        ctr_Governor();
        stabilize();
        if (var.altitude <= apcfg.ld_flareAlt)
            stage++;
    } break;
    case LANDING_stage::Flare: {
        var.cmode &= ~cmode_hyaw;
        if (conf.ahrs_noMAG != ahrs_noMAG_off)
            var.cmode |= cmode_nomag;
    } break;
    case LANDING_stage::Flare_do: {
        setNE(rwNE
              + var.rotate(Point(appLength * var.limit(var.altitude / apcfg.altitude, 0, 1), 0),
                           -(180.0 + rwHDG)));
        ctr_HoverX();
        ctr_HoverY();
        set_yaw(var.cmd_course, false); //user input only
        set_vspeed(-apcfg.ld_flareDS);
        ctr_VSpeed();
        ctr_Governor();
        stabilize();
        //check if collective below initial
        if (apcfg.ld_tdEvent == ld_tdEvent_altitude
            || (apcfg.ld_tdEvent == ld_tdEvent_agl && apcfg.ld_AGL == 0)) {
            if (!(var.ctr_collective < apcfg.to_liftPitch && var.vspeed > (-apcfg.ld_flareDS / 2.0)))
                time_s = t;
            else if ((t - time_s) > 1000)
                stage++;
        }
    } break;
    case LANDING_stage::Flare_done: {
        stage = LANDING_stage::Touchdown;
    } break;
    case LANDING_stage::Touchdown: {
        if (var.ctr_collective > apcfg.to_liftPitch) {
            if (apcfg.ld_brkSpeed <= 0) {
                var.ctr_collective = apcfg.to_liftPitch;
            } else {
                var.ctr_collective -= 0.1;
                if (var.ctr_collective < -1)
                    var.ctr_collective = -1;
            }
        }
        var.ctr_brake = 1;
    } break;
    case LANDING_stage::Touchdown_do: {
        var.calc();
        reg_HoverX.reset();
        reg_HoverY.reset();
        ctr_HoverX();
        ctr_HoverY();
        set_yaw(var.cmd_course, false); //user input only
        stabilize();
        //decrease collective slowly
        if (var.ctr_collective > apcfg.to_liftPitch) {
            var.ctr_collective -= apcfg.ld_brkSpeed * dt;
            if (var.ctr_collective < apcfg.to_liftPitch) {
                var.ctr_collective = apcfg.to_liftPitch;
            }
        } else
            stage = LANDING_stage::Run;
    } break;
    case LANDING_stage::Run: {
        if (apcfg.ld_tdEngine == ld_tdEngine_ignOff) {
            var.cmode |= cmode_thrcut;
            var.power &= ~power_ignition;
        } else if (apcfg.ld_tdEngine == ld_tdEngine_thrCut) {
            var.cmode |= cmode_thrcut;
            var.cmode |= cmode_throvr;
        }
        if (var.ctr_collective > apcfg.to_liftPitch) {
            var.ctr_collective = apcfg.to_liftPitch;
        }
        var.calc();
        set_thr(0);
        var.ctr_brake = 1;
        reg_HoverX.reset();
        reg_HoverY.reset();
        ctr_HoverX();
        ctr_HoverY();
        set_yaw(var.cmd_course, false); //user input only
        stabilize();
        stage = LANDING_stage::Finished;
    } break;
    case LANDING_stage::Run_do: {
        stage = LANDING_stage::Finished;
    } break;
    case LANDING_stage::Run_done: {
        stage = LANDING_stage::Finished;
    } break;
    case LANDING_stage::Cancel: {
        if (conf.ahrs_noMAG != ahrs_noMAG_always)
            var.cmode &= ~cmode_nomag;
        var.ctr_brake = 0;
        var.power &= ~power_agl;
        if (var.cmd_altitude < apcfg.altitude)
            var.cmd_altitude = apcfg.altitude;
    } break;
    case LANDING_stage::Cancel_do: {
        var.calc();
        ctr_HoverX();
        ctr_HoverY();
        set_yaw(var.cmd_course, false); //user input only
        ctr_Altitude();
        ctr_VSpeed();
        ctr_Governor();
        stabilize();
        if (var.altitude > (apcfg.altitude * 0.8))
            stage++;
    } break;
    case LANDING_stage::Cancel_done: {
    } break;
    case LANDING_stage::Finished: {
        var.ctr_brake = 1;
        var.power &= ~power_agl;
        if (conf.ahrs_noMAG != ahrs_noMAG_always)
            var.cmode &= ~cmode_nomag;
        if (var.ctr_collective > apcfg.to_liftPitch)
            var.ctr_collective = apcfg.to_liftPitch;
    } break;
    case LANDING_stage::Finished_do: {
        var.calc();
        set_roll(var.rc_roll * 15.0, false);
        set_pitch(var.rc_pitch * 15.0, false); //set by rc adjust
        stabilize();
        reg_Roll.reset();
        reg_Pitch.reset();
        set_yaw(var.cmd_course, false); //user input only
        ctr_Yaw();
        set_thr(0);
    } break;
    case LANDING_stage::Finished_done: {
    } break;
    }
    return stage;
}
//==============================================================================
uint32_t Shiva::LANDING_blimp(uint32_t stage)
{
    switch (stage) {
    case LANDING_stage::init: {
    } break;
    case LANDING_stage::TA: {
    } break;
    case LANDING_stage::TA_do: {
    } break;
    case LANDING_stage::TAapp: {
    } break;
    case LANDING_stage::TAapp_do: {
    } break;
    case LANDING_stage::App: {
    } break;
    case LANDING_stage::App_do: {
    } break;
    case LANDING_stage::Flare: {
    } break;
    case LANDING_stage::Flare_do: {
    } break;
    case LANDING_stage::Flare_done: {
    } break;
    case LANDING_stage::Touchdown: {
    } break;
    case LANDING_stage::Touchdown_do: {
    } break;
    case LANDING_stage::Run: {
    } break;
    case LANDING_stage::Run_do: {
    } break;
    case LANDING_stage::Run_done: {
    } break;
    case LANDING_stage::Cancel: {
    } break;
    case LANDING_stage::Cancel_do: {
    } break;
    case LANDING_stage::Cancel_done: {
    } break;
    case LANDING_stage::Finished: {
    } break;
    case LANDING_stage::Finished_do: {
    } break;
    case LANDING_stage::Finished_done: {
    } break;
    }
    return stage;
}
//==============================================================================
uint32_t Shiva::LANDING_boat(uint32_t stage)
{
    switch (stage) {
    case LANDING_stage::init: {
    } break;
    case LANDING_stage::TA: {
    } break;
    case LANDING_stage::TA_do: {
    } break;
    case LANDING_stage::TAapp: {
    } break;
    case LANDING_stage::TAapp_do: {
    } break;
    case LANDING_stage::App: {
    } break;
    case LANDING_stage::App_do: {
    } break;
    case LANDING_stage::Flare: {
    } break;
    case LANDING_stage::Flare_do: {
    } break;
    case LANDING_stage::Flare_done: {
    } break;
    case LANDING_stage::Touchdown: {
    } break;
    case LANDING_stage::Touchdown_do: {
    } break;
    case LANDING_stage::Run: {
    } break;
    case LANDING_stage::Run_do: {
    } break;
    case LANDING_stage::Run_done: {
    } break;
    case LANDING_stage::Cancel: {
    } break;
    case LANDING_stage::Cancel_do: {
    } break;
    case LANDING_stage::Cancel_done: {
    } break;
    case LANDING_stage::Finished: {
    } break;
    case LANDING_stage::Finished_do: {
    } break;
    case LANDING_stage::Finished_done: {
    } break;
    }
    return stage;
}
//==============================================================================
bool Shiva::adjust_landing_circle()
{
    if ((t - time_VE) < 107)
        return last_VE_result;
    time_VE = t;

    //10Hz
    const _var_float snap = var.cmd_airspeed;
    last_VE_result = true;
    get_landing_length(var.cmd_altitude);
    const _var_float adj = var.limit(var.delta / var.cmd_airspeed, var.cmd_airspeed) * 0.5;
    if (var.delta > 0) {
        //too close
        if (cSpan > 180)
            cR += adj; ///(2.0*M_PI);
        else
            appLength += PI * adj;
    } else if (var.delta < (-snap)) {
        //too far, decrease circle
        if (appLength > aLen)
            appLength += adj;
        else
            cR += adj;
        if (cR <= apcfg.turnR) {
            cR = apcfg.turnR;
            if (appLength > aLen) { //approach line was increased before
                if (cSpan < 30)
                    last_VE_result = false;
            } else {
                if (cSpan < 80 && (var.delta < (-snap)))
                    last_VE_result = false;
            }
        }
    }
    const _var_float cR_max = apcfg.turnR * 5.0;
    if (cR > cR_max)
        cR = cR_max;
    if (cSpan < 15 && var.delta < aLen)
        last_VE_result = false;
    var.turnR = rwLeft ? -cR : cR;
    return last_VE_result;
}
//==============================================================================
void Shiva::get_landing_length(_var_float gnd_altitude)
{
    cNE = rwNE + var.rotate(appLength, rwLeft ? cR : -cR, -(180.0 + rwHDG));
    const Point cuNE = cNE - var.pos_NE;
    //distance from UAV to circle
    _var_float dCircle0 = var.distance(cuNE);
    _var_float dCircle = dCircle0;
    if (dCircle <= cR)
        dCircle = cR - dCircle; //inside circle
    else
        dCircle -= cR;
    //arc parameters
    const _var_float cHdg = var.heading(cuNE); //heading from UAV to circle center
    const _var_float arc_start = var.boundAngle360(
        cHdg + (rwLeft ? 90.0 : -90.0)); //circle flight start course
    cSpan = var.boundAngle360(rwLeft ? (arc_start - var.boundAngle360(rwHDG))
                                     : (var.boundAngle360(rwHDG) - arc_start));
    //count whole path length corrected with wind triangle (longer path in air mass)
    _var_float way = 0;
    //if(dCircle>cR && arc_start>90) way+=dCircle/var.wind_triangle(cHdg); //from uav to circle
    //dist to circle
    //Point tuNE=find_landing_tangent()-var.pos_NE;
    //way+=var.distance(tuNE)/var.wind_triangle(var.heading(tuNE));
    if (cSpan > (180 + 45) || dCircle0 <= cR) {
        //inside circle
        way += dCircle / var.wind_triangle(var.course); //dCircle0<=cR?cHdg+180:cHdg);//
    }
    //var.user1=way;
    //arc way
    _var_float arc_way = var.wind_circle(arc_start,
                                         rwLeft ? -std::abs(cSpan) : std::abs(cSpan),
                                         cR); //circle arc
    way += arc_way;
    app_Kwt = var.wind_triangle(rwHDG);
    way += appLength / app_Kwt; //approach to runway
    vLE_est = way;
    vLE_total = (var.altitude - gnd_altitude) * var.ldratio;
    setDelta(vLE_total - vLE_est);
    var.cmd_NE = cNE;
}
//==============================================================================
void Shiva::setDelta(_var_float delta)
{
    // var.user1=delta;
    if (var.delta == 0) {
        delta_kf.reset(delta);
        var.delta = delta;
    } else {
        delta_kf.step(delta, &(var.delta), 0.1, 15);
        if (std::isinf(var.delta) || std::isnan(var.delta)) {
            delta_kf.reset(delta);
            var.delta = delta;
        }
    }
}
//==============================================================================
Point Shiva::find_landing_tangent(void)
{
    //find a tangent point to min landing circle
    _var_float R = cR;    //apcfg.turnR*1.5; //circle radius
    const Point &c = cNE; //(rwNE+var.rotate(appLength,R,-(rwHDG+180.0))); //circle center
    const Point &p = var.pos_NE;
    Point tp;
    const Point dp = p - c;
    if (var.distance(dp) <= R * 1.5) {
        _var_float a = (var.heading(dp, true) + (rwLeft ? 30 : -30)) * D2R;
        tp = c + R * Point(std::sin(a), std::cos(a));
    } else {
        _var_float Xc = c[1], Yc = c[0], Xo = p[1], Yo = p[0];
        _var_float A, B, C, k1, R2 = R * R;
        A = 2 * Xc * Xo - Xc * Xc + R2 - Xo * Xo;
        B = Xc * Yc - Xc * Yo + Yo * Xo - Xo * Yc;
        C = R2 - Yc * Yc - Yo * Yo + 2 * Yc * Yo;
        k1 = (-B + std::sqrt(B * B - A * C)) / A; //k2=-(-B+std::sqrt(B*B-A*C))/A;
        if (!rwLeft)
            k1 = -k1;
        tp[1] = (Xc + k1 * (Yc + k1 * Xo - Yo)) / (k1 * k1 + 1);
        tp[0] = k1 * (tp[1] - Xo) + Yo;
    }
    return tp;
}
//==============================================================================
