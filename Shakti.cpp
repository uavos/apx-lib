//==============================================================================
#include <math.h>
#include <string.h>
//--------------------------------------------------------
#include "Shakti.h"
#include "bus.h"
#include "time_ms.h"
#include <Mandala>
//==============================================================================
void Shakti::init()
{
    mission.clear();

    if (var.ldratio <= 0)
        var.ldratio = apcfg.ldratioMin;

    counter = 0;

    power_on_reset = true;
    rc_timeout = link_timeout = 0;
    rpm_time_s = 0;
    rpm_s = 0;
    rpmv_s = 0;
    rpm_cnt = 0;

    last_wpHDG = 0;
    wpHDG_cnt = 0;

    gps_jstate_s = 0;

    venergy_mean = 0;
    last_brake = 0;

    safety_Ve = safety_Vs = safety_VeERS = false;
    wrn_mask = 0;

    time_eta_s = 0;
    last_dWPT = 0;
    dot_dWPT = 0;

    cam_mode_s = -1;

    area.stage = 0;

    starter_stage = 0;
    var.ctrb &= ~ctrb_starter;

    var.cmd_altitude = apcfg.altitude;
    var.cmd_airspeed = apcfg.spd_cruise;
    var.power = power_ap; //|power_servo|power_ignition;
    var.cmode |= cmode_thrcut;

    doAutoFlaps = true;

    switch (apcfg.init_mode) {
    case init_mode_TAXI:
        var.mode = mode_TAXI;
        break;
    case init_mode_TAKEOFF:
        var.mode = mode_TAKEOFF;
        break;
    case init_mode_EMG:
        var.mode = mode_EMG;
        break;
    default:
        var.mode = mode_TAXI;
    }
    if (apcfg.init_ignition == init_ignition_off)
        var.power &= ~power_ignition;
    if (apcfg.init_servo == init_servo_off)
        var.power &= ~power_servo;
    if (apcfg.init_thrcut == init_thrcut_off)
        var.cmode &= ~cmode_thrcut;
    else if (apcfg.init_thrcut == init_thrcut_override)
        var.cmode |= cmode_throvr;

    if (apcfg.fuel > 0)
        var.fuel = apcfg.fuel;

    //send_update_vars();
    last_autoflaps = 0;

    ctr_brake_s = var.ctr_brake;
    ctr_flaps_s = var.ctr_flaps;
    sb_s = var.sb;

    mode_s = -1;
}
//==============================================================================
Shakti::Shakti()
    : reg_Roll(&apcfg.Roll)
    , reg_Pitch(&apcfg.Pitch)
    , reg_Yaw(&apcfg.Yaw)
    , reg_Thr(&apcfg.Thr)
    , reg_Airspeed(&apcfg.Airspeed)
    , reg_Altitude(&apcfg.Altitude)
    , reg_VSpeed(&apcfg.VSpeed)
    , reg_Course(&apcfg.Course)
    , reg_Line(&apcfg.Line)
    , reg_Path(&apcfg.Path)
    , reg_Slip(&apcfg.Slip)
    , reg_Governor(&apcfg.Governor)
    , reg_Thermal(&apcfg.Thermal)
    , reg_Taxi(&apcfg.Taxi)
    , reg_Brakes(&apcfg.Brakes)
    , reg_HoverX(&apcfg.HoverX)
    , reg_HoverY(&apcfg.HoverY)
    , reg_RpmPitch(&apcfg.RpmPitch)
    , reg_AirbrkApp(&apcfg.ld_AirbrkApp)
    , reg_AirbrkDS(&apcfg.ld_AirbrkDS)
{
    init();
}
//==============================================================================
//==============================================================================
void Shakti::fly(_var_float a_dt)
{
    dt = a_dt;
    t = time;
    if (dt == 0)
        dt = 1.0 / 100.0;

    checkModeChange(); //if changed by GCU
    //send_update_vars();

    //------------------------
    maneuver(); //virtual
    checkRestrictedAreas();
    safety();
    control_cam();
    //------------------------

    //dynamics smooth
    smooth_cmd_airspeed();

    //diff brakes
    if (apcfg.dbrk_steering > 0) {
        if (var.ctr_brake > 0) {
            var.ctr_brakeL = var.ctr_brakeR = var.ctr_brake;
        } else if (fabs(var.ctr_steering) > apcfg.dbrk_steering) {
            float err = (fabs(var.ctr_steering) - apcfg.dbrk_steering)
                        / (1.0 - apcfg.dbrk_steering);
            float v = err * apcfg.dbrk_Kp;
            var.ctr_brakeL = var.ctr_steering > 0 ? 0 : var.limit(v, 0, 1);
            var.ctr_brakeR = var.ctr_steering < 0 ? 0 : var.limit(v, 0, 1);
        } else {
            var.ctr_brakeL = var.ctr_brakeR = 0;
        }
    } else {
        var.ctr_brakeL = var.ctr_brakeR = var.ctr_brake;
    }

    //starter procedure
    starter();

    //maintain reasonable ldratio
    var.ldratio = var.limit(var.ldratio, apcfg.ldratioMin, apcfg.ldratioMax);

    //calc venergy mean
    var.filter_m(var.venergy - apcfg.therm_b, &venergy_mean, apcfg.therm_flt * dt);

    //calc stability
    calc_stab();

    //------------------------
    //RC control override
    if (rc_timeout && ((--rc_timeout) == 0))
        rc_updated(false); //RC signal disappeared

    if (var.status & status_rc) {
        //avoid transition shake
        var.cmd_theta[0] = var.theta[0];
        var.cmd_theta[1] = var.theta[1];
        set_thr(var.rc_throttle);
    }

    //------------------------
    //Data link status check
    if (link_timeout && ((--link_timeout) == 0))
        link_updated(false); //data link signal disappeared

    //------------------------
    //RPM status check
    if ((t - rpm_time_s) > 10000) {
        rpm_updated(false);
    }

    //------------------------
    //print some (var change) status messages

    if (apcfg.flaps_auto == 0) {
        print_status_float(idx_ctr_flaps, &ctr_flaps_s);
    }

    if (ctr_brake_s > 0 && var.ctr_brake > 0 && var.ctr_brake != 1) {
        ctr_brake_s = var.ctr_brake; //don't print if not zero
    }
    print_status_float(idx_ctr_brake, &ctr_brake_s);

    print_status_enum(idx_gps_jstate, gps_jstate_s);
    gps_jstate_s = var.gps_jstate;

    //Event sensors logic (power control)
    print_status_flag(idx_sb, sb_s);
    sb_s = var.sb;
    if (var.sb & sb_shutdown) {
        if ((t - time_shdn_s) > 200)
            var.power = 0;
    } else
        time_shdn_s = t;

    //fuel rate to capacity
    if (apcfg.frate > 0) {
        if (var.ctr_throttle > 0)
            var.frate = apcfg.frate * var.ctr_throttle;
        else
            var.frate = 0;
    }
    if (var.fuel > 0 && var.frate > 0 && (t - time_frate) >= 500) { //counter%(AHRS_FREQ/2)==0){
        time_frate = t;
        var.fuel -= var.frate / (3600.0 * 2.0);
        if (var.fuel < 0)
            var.fuel = 0;
    }

    checkModeChange(); //if changed by Shiva

    /*send_update_vars();

  send_downstream(counter);

  if (!(counter%(AHRS_FREQ/(AHRS_FREQ==100?(CTR_FREQ/2):CTR_FREQ))))
    send(idx_ctr);

  if (!(counter%(AHRS_FREQ/20)))
    send(idx_theta);*/

    power_on_reset = false;
    counter++;
}
//==============================================================================
void Shakti::calc_stab(void)
{
    if (apcfg.stab_est <= 0)
        return;
    _var_float vstab = (var.theta[0] - var.cmd_theta[0]);
    //turn
    _var_float spd = var.cmd_airspeed * var.cas2tas;
    _var_float turn_rate = spd > 0 ? R2D * 9.81 * tan(var.cmd_theta[0] * D2R) / spd : 0;
    turn_rate = (var.gyro[2] - turn_rate);
    //var.user1=turn_rate;
    if (apcfg.stab_opt == stab_opt_HPF) {
        if (fabs(turn_rate) < 1)
            f_stab_turn.reset(turn_rate);
        turn_rate = f_stab_turn.step(turn_rate, dt, dt * 100.0);
    }
    //var.user2=turn_rate;
    vstab *= turn_rate;
    //venergy
    _var_float vs = var.venergy - var.vspeed;
    if (vs >= 0) {
        vs = vs + 1;
        vstab /= vs * vs;
    } else {
        vs = -vs + 1;
        vstab *= vs * vs;
    }
    //limit & filter
    vstab = fabs(vstab);
    if (apcfg.stab_opt == stab_opt_airspeed)
        vstab *= apcfg.spd_cruise - var.airspeed;
    vstab = (vstab <= 0) ? 0.0 : var.limit(1.0 - apcfg.stab_est / vstab, 0, 1);
    var.filter_m(vstab, &var.stab, dt);
}
//==============================================================================
void Shakti::rc_updated(bool valid)
{
    if (valid) {
        rc_timeout = 0.5 / dt;
        var.status |= status_rc;
    } else {
        rc_timeout = 0;
        var.status &= ~status_rc;
        if (var.mode <= mode_UAV)
            var.mode = mode_HOME;
    }
}
void Shakti::link_updated(bool valid)
{
    if (valid) {
        link_timeout = (apcfg.sfdl_timeout < 10 ? 10.0 : (_var_float) apcfg.sfdl_timeout) / dt;
        var.status |= status_modem;
    } else if (var.status & status_modem) {
        link_timeout = 0;
        var.status &= ~status_modem;
        if (var.mode != mode_TAXI && var.mode != mode_TAKEOFF && var.mode != mode_LANDING) {
            switch (apcfg.sfdl_action) {
            case sfdl_action_off:
                break;
            case sfdl_action_landing:
                var.mode = mode_LANDING;
                break;
            case sfdl_action_ERS:
                var.ctrb |= ctrb_ers;
                break;
            }
        }
    }
}
void Shakti::rpm_updated(bool valid)
{
    if (valid) {
        uint d = var.rpm >= rpm_s ? var.rpm - rpm_s : rpm_s - var.rpm;
        rpm_s = var.rpm;

        if (apcfg.rpm_pflt && apcfg.rpm > 0) {
            //filter RPM peaks
            const uint ds = apcfg.rpm
                            / (apcfg.rpm_pflt == rpm_pflt_p8
                                   ? 8
                                   : apcfg.rpm_pflt == rpm_pflt_p4 ? 4 : 2);
            if (d > ds && rpm_cnt < 5) {
                rpm_cnt++;
                var.rpm = rpmv_s;
            } else {
                rpm_cnt = 0;
                rpmv_s = var.rpm;
            }
        }

        if (var.rpm > 0) {
            if (apcfg.rpm_err != rpm_err_derivative || d > 0) {
                rpm_time_s = time;
            }
            var.error &= ~error_rpm;
            return;
        }
    }
    //rpm error
    if (rpm_time_s)
        var.error |= error_rpm;
}
//==============================================================================
void Shakti::safety()
{
    if (apcfg.frm_mode == frm_mode_wingC)
        return;

    //Voltages
    const _var_float hystV = 1.2;
    bool bVerr = false;
    //Voltage Ve
    if (t > 5000) {
        if (!safety_Ve) {
            if ((apcfg.sf_Ve != 0) && (var.Ve > (apcfg.sf_Ve * 0.3)) && (var.Ve <= apcfg.sf_Ve)) {
                dmsg("Voltage error: Ve %.1fv (<%.1fv)\n", var.Ve, apcfg.sf_Ve);
                safety_Ve = bVerr = true;
            }
        } else if (var.Ve > (apcfg.sf_Ve * hystV))
            safety_Ve = false;
        //Voltage Vs
        if (!safety_Vs) {
            if ((apcfg.sf_Vs != 0) && (var.Vs > (apcfg.sf_Vs * 0.3)) && (var.Vs <= apcfg.sf_Vs)) {
                dmsg("Voltage error: Vs %.1fv (<%.1fv)\n", var.Vs, apcfg.sf_Vs);
                safety_Vs = bVerr = true;
            }
        } else if (var.Vs > (apcfg.sf_Vs * hystV))
            safety_Vs = false;
        if (bVerr)
            var.error |= error_power;
    }

    if (var.mode != mode_EMG && var.mode != mode_TAKEOFF && var.mode != mode_LANDING
        && var.mode != mode_TAXI) {
        //dHome
        _var_float v_dHome = apcfg.sf_dHome;
        if ((var.mode != mode_HOME) && (v_dHome != 0) && (var.dHome > v_dHome)) {
            dmsg("Error: Too far distance (return) %.1f km\n", var.dHome / 1000.0);
            var.mode = mode_HOME;
        }
        //dHomeERS
        _var_float v_dHomeERS = apcfg.sf_dHomeERS;
        if ((!(var.ctrb & ctrb_ers)) && (v_dHomeERS != 0) && (var.dHome > v_dHomeERS)) {
            dmsg("Error: Too far distance (ers) %.1f km\n", var.dHome / 1000.0);
            var.ctrb |= ctrb_ers;
        }
        //vspeedERS
        if ((!(var.ctrb & ctrb_ers)) && (apcfg.sf_vspeedERS != 0)
            && (var.vspeed < -apcfg.sf_vspeedERS)
            && ((apcfg.altitude < 50) || (var.altitude <= (apcfg.altitude / 2.0)))) {
            dmsg("Error: Variometer overspeed %i m/s\n", (int) var.vspeed);
            var.ctrb |= ctrb_ers;
        }
        if (bVerr && apcfg.sfVe_action) {
            switch (apcfg.sfVe_action) {
            case sfVe_action_landing:
                var.mode = mode_LANDING;
                break;
            }
        }
        //Voltage Ve ERS
        if (!safety_VeERS) {
            if ((apcfg.sf_VeERS != 0) && (var.Ve > (apcfg.sf_VeERS * 0.3))
                && (var.Ve <= apcfg.sf_VeERS)) {
                dmsg("Voltage error: Ve %.1fv (<%.1fv)\n", var.Ve, apcfg.sf_VeERS);
                safety_VeERS = true;
                var.ctrb |= ctrb_ers;
            }
        } else if (var.Ve > (apcfg.sf_VeERS * hystV))
            safety_VeERS = false;
    }

    if (var.ctrb & ctrb_ers) {
        //safe throttle cut off when ERS
        set_thr(0);
        var.cmode |= cmode_thrcut;
        var.power &= ~power_ignition;
        if (var.mode != mode_TAKEOFF && var.mode != mode_TAXI)
            var.mode = mode_LANDING; //for parachute release when done
    }

    //warnings
    if ((t - time_wrn_s) >= 1000) {
        time_wrn_s = t;
        uint32_t msk = 1;
        uint32_t warn = 0;
        for (uint i = 0; i < (sizeof(apcfg.wrn_bind) / sizeof(apcfg.wrn_bind[0])); i++) {
            if (apcfg.wrn_bind[i]) {
                _var_float v = var.get_data(apcfg.wrn_bind[i]);
                if (v < apcfg.wrn_min[i])
                    warn |= msk;
                else if (v > apcfg.wrn_max[i])
                    warn |= msk;
                while ((warn ^ wrn_mask) & msk & warn) { //changed low-high
                    //print warning var descr
                    uint16_t var_m = apcfg.wrn_bind[i];
                    uint type;
                    void *value_ptr;
                    if (!var.get_ptr(var_m, &value_ptr, &type))
                        break; //get var type
                    const char *name;
                    const char *descr;
                    if (type == vt_enum)
                        var_m |= 0xFF00;
                    if (!var.get_text_names(var_m, &name, &descr))
                        break;
                    dmsg("Warning: %s\n", descr);
                    break;
                }
            }
            msk <<= 1;
        }
        wrn_mask = warn;
    }
}
//==============================================================================
void Shakti::checkRestrictedAreas(bool reset)
{
    //worker state machine to scan all restricted areas
    if (reset)
        area.stage = 0;
    switch (area.stage++) {
    case 0:
        //init before scan all obstacles
        area.idx = 0;
        area.minDist = 0;
        area.isInside = false;
        area.sum = 0;
        return;
    case 1:
        if (apcfg.sf_NoFlyZones == 0) {
            area.stage = 1000;
            return;
        }
        area.area = mission.restricted(area.idx);
        if (area.area && area.area->pointsCnt >= 2)
            return;
        if (area.idx) {
            //all obstacles scanned
            area.out.dist = area.minDist;
            area.out.intersect = area.intersect;
            area.out.isInside = area.isInside;
            area.out.sum = area.sum;
            /*var.user1=area.minDist;
        var.user2=area.out.sum[0];
        var.user3=area.out.sum[1];
        var.user4=area.isInside;*/
            area.stage = 0;
        } else
            area.stage = 1000; //wait mission update
        return;
    case 2: //find closest distance to all lines
        area.pidx = 0;
        area.inside_c = false;
        area.poly_sum = 0;
        area.poly_cnt = 0;
        //area.minDist=0;
        area.point = mission.area_point(area.area, area.pidx++);
        return;
    case 3: {
        Mission::_item_area_point *p = mission.area_point(area.area,
                                                          area.pidx == area.area->pointsCnt
                                                              ? 0
                                                              : area.pidx);
        area.pidx++;
        area.p1 = var.llh2ne(Vect(area.point->lat, area.point->lon, var.gps_pos[2]), var.gps_pos);
        area.p2 = var.llh2ne(Vect(p->lat, p->lon, var.gps_pos[2]), var.gps_pos);
        area.point = p;
    }
        return;
    case 4: {
        Point pd = var.lineDistance(area.p1, area.p2);
        float dist2 = pd.mag2();
        float dist = sqrt(dist2);
        //update sum of vectors
        area.poly_sum -= (pd / dist2);
        area.poly_cnt++;
        //calc min distance
        if (area.minDist == 0 || dist < area.minDist) {
            area.intersect = pd; //intersect point
            area.minDist = dist;
        }
    }
        return;
    case 5: {
        //check inside or not
        if (((area.p1[1] > 0) != (area.p2[1] > 0))
            && (0 < (area.p2[0] - area.p1[0]) * (-area.p1[1]) / (area.p2[1] - area.p1[1])
                        + area.p1[0]))
            area.inside_c = !area.inside_c;
        //continue next line
        if (area.pidx <= area.area->pointsCnt) {
            area.stage = 3;
            return;
        }
        //found minimum distance to one area, try others
        area.sum += area.poly_sum / (float) area.poly_cnt;
        area.isInside |= area.inside_c;
        area.idx++;
        area.stage = 1; //0
    }
        return;

    case 1000:
        memset(&area.out, 0, sizeof(area.out));
    case 1001:
        area.stage = 1001;
        return;
    default:
        area.stage = 0;
    }
}
//==============================================================================
void Shakti::starter()
{
    if (var.ctrb & ctrb_starter) {
        var.ctrb &= ~ctrb_starter;
        if (starter_stage == 0)
            starter_stage = 1;
    }
    if (starter_stage == 0) {
        if (var.sw & sw_starter) {
            starter_stage = 5;
            time_starter_s = t;
            dmsg("Engine start forced\n");
        } else
            return;
    }
    if (var.stage == 100) {
        dmsg("Engine start cancelled\n");
        starter_stage = 0; //cancel
    }
    switch (starter_stage) {
    case 1: //check no RPM present
        var.sw &= ~sw_starter;
        if (var.power & power_ignition)
            dmsg("Engine start procedure\n");
        else
            dmsg("Engine cold turn procedure\n");
        if (var.rc_throttle > 0.4) {
            dmsg("Error: throttle not in zero\n");
            break;
        }
        if (var.rc_throttle != var.ctr_throttle) {
            dmsg("Error: auto throttle is on\n");
            break;
        }
        if (!(var.power & power_servo)) {
            dmsg("Error: servo power is off\n");
            break;
        }
        if (apcfg.st_choke && (var.power & power_ignition))
            var.ctr_mixture = 1;
        var.ctrb |= ctrb_horn;
        time_starter_s = t;
        starter_stage++;
    case 2:
        if ((!(var.error & error_rpm)) && var.rpm > 0) {
            dmsg("Error: Engine is already running\n");
            break;
        }
        var.ctr_throttle = apcfg.st_thr;
        if ((t - time_starter_s) < 1000)
            return;
        time_starter_s = t;
        starter_stage++;
    case 3:
        var.ctr_throttle = apcfg.st_thr;
        if ((t - time_starter_s) < (apcfg.st_pause * 1000))
            return;
        starter_stage++;
    case 4: //turn on switch for period
        var.sw |= sw_starter;
        var.ctrb &= ~ctrb_horn;
        time_starter_s = t;
        starter_stage++;
    case 5: //check if engine started
        var.ctr_throttle = apcfg.st_thr;
        if (apcfg.rpm_idle > 0 && (!(var.error & error_rpm)) && var.rpm >= apcfg.rpm_idle) {
            starter_stage++;
            return;
        } else {
            uint to = apcfg.st_time * 1000;
            if (to < 1000)
                to = 1000;
            if ((t - time_starter_s) < to)
                return;
        }
        dmsg("Error: Engine starter timeout\n");
        break;
    case 6: //warm-up
        var.sw &= ~sw_starter;
        if (apcfg.st_choke) {
            time_starter_s = t;
            starter_stage++;
            return;
        }
        starter_stage = 100;
        return;
    case 7: //choke time
        if ((t - time_starter_s) < (apcfg.st_choke * 1000))
            return;
        dmsg("Engine choke off\n");
        break;
    case 100:
        dmsg("Engine start finished\n");
        break;
    }
    starter_stage = 0;
    set_thr(0);
    var.sw &= ~sw_starter;
    var.ctrb &= ~ctrb_horn;
    if (apcfg.st_choke)
        var.ctr_mixture = 0;
}
//==============================================================================
//==============================================================================
void Shakti::setNE(const Point &ne)
{
    var.cmd_NE = ne;
    var.calc();
}
//==============================================================================
void Shakti::stabilize()
{
    switch (apcfg.type) {
    case type_airplane:
        ctr_Roll();
        ctr_Pitch();
        if (!ctr_Slip())
            set_rud(var.rc_yaw); // mix
        //set_steering(var.rc_yaw);
        break;
    case type_helicopter:
        ctr_Roll();
        ctr_Pitch();
        ctr_Yaw();
        break;
    case type_blimp:
        ctr_Pitch(); //elevator
        ctr_Yaw();
        if (var.mode != mode_EMG) {
            if (var.buoyancy < (apcfg.blimp_buoyancy - apcfg.blimp_buoyancy_hyst)) {
                var.ctr_buoyancy = 1; //turn on pump
            } else if (var.buoyancy > (apcfg.blimp_buoyancy + apcfg.blimp_buoyancy_hyst)) {
                var.ctr_buoyancy = -1; //turn on valve
            } else {
                var.ctr_buoyancy = 0; //turn off pump and valve
            }
        }
        break;
    case type_boat:
        //nothing
        break;
    }
}
//==============================================================================
void Shakti::navigate(bool do_stabilize)
{
    switch (apcfg.type) {
    case type_airplane:
        set_airspeed(var.cmd_airspeed);
        ctr_Course();
        ctr_Altitude();
        ctr_Airspeed();
        ctr_Thr();
        ctr_Governor();
        ctr_autoflaps();
        break;
    case type_helicopter:
        if (var.cmode & cmode_hover) {
            ctr_HoverX();
            ctr_HoverY();
            set_yaw(var.cmd_course);
        } else {
            ctr_Course();
            ctr_Airspeed();
            reg_HoverX.reset(var.cmd_theta[1]);
            reg_HoverY.reset();
        }
        ctr_Altitude();
        ctr_VSpeed();
        if (!ctr_Governor())
            set_thr(var.rc_throttle);
        break;
    case type_blimp:
        ctr_Course();
        ctr_Altitude();
        ctr_VSpeed();
        ctr_Thr();
        break;
    case type_boat:
        set_yaw(var.cmd_course);
        ctr_Course();
        ctr_Thr();
        break;
    }
    if (do_stabilize)
        stabilize();
}
//==============================================================================
//==============================================================================
bool Shakti::flyto(const Point &ne, _var_float sdist)
{
    setNE(ne);
    var.cmd_course = var.wpHDG;
    return check_wp_reached(sdist);
}
//==============================================================================
bool Shakti::flytoHover(const Point &ne, _var_float sdist)
{
    if (var.cmode & cmode_hover) {
        var.calc();
        return true;
    }
    if (flyto(ne, sdist) && fabs(var.boundAngle(var.course - var.cmd_course)) < 45) {
        var.cmode |= cmode_hover;
        var.cmd_course = var.tgHDG;
        var.cmd_airspeed = 0;
        return true;
    } else
        var.cmd_airspeed = var.limit(apcfg.HoverX.Lpp + (var.dWPT - sdist) * apcfg.heli_Khs,
                                     apcfg.HoverX.Lpp,
                                     var.cmd_airspeed);
    return false;
}
//==============================================================================
bool Shakti::flytoLine(const Point &ne, _var_float sdist)
{
    setNE(ne);
    ctr_Line();
    return check_wp_reached(sdist);
}
//==============================================================================
void Shakti::flytoCircle(const Point &ne, _var_float r, bool bCCW, bool adj90)
{
    setNE(ne);
    if (bCCW) {
        var.rwDelta = var.dWPT - r;
        var.tgHDG = var.boundAngle(var.wpHDG + 90.0);
    } else {
        var.rwDelta = r - var.dWPT;
        var.tgHDG = var.boundAngle(var.wpHDG - 90.0);
    }
    ctr_Path();
    //if(adj90){
    _var_float sdist = apcfg.turnR * 2;
    if (var.rwDelta > sdist)
        var.cmd_course = var.boundAngle(var.tgHDG - 90);
    else if (var.rwDelta < -sdist)
        var.cmd_course = var.boundAngle(var.tgHDG + 90);
    //}
}
//==============================================================================
bool Shakti::check_wp_reached(_var_float sdist)
{
    //default is snap to distance
    if (sdist >= 0 || apcfg.wpt_drv == 0)
        return var.dWPT <= (sdist < 0 ? apcfg.wpt_snap : sdist);

    //calculate wpHDG derivative
    if ((counter % (int) (0.2 / dt)) == 0) {
        _var_float d = var.boundAngle(var.wpHDG - last_wpHDG) / 0.2;
        last_wpHDG = var.wpHDG;
        if (fabs(d) >= apcfg.wpt_drv)
            wpHDG_cnt++;
        else
            wpHDG_cnt = 0;
        if (wpHDG_cnt && var.dWPT <= apcfg.spd_cruise) {
            //very close to waypoint
            wpHDG_cnt = 0;
            return true;
        }
        if (wpHDG_cnt < 3)
            return false;
        wpHDG_cnt = 0;
        return var.dWPT <= apcfg.turnR;
    }
    return false;
}
//==============================================================================
//==============================================================================
void Shakti::checkModeChange(void)
{
    if (apcfg.frm_mode == frm_mode_wingL || apcfg.frm_mode == frm_mode_wingR) {
        var.mode = mode_RPV;
        return;
    }
    //reset some vars if flight mode changed
    if (mode_s == var.mode)
        return;
    mode_s = var.mode;
    var.stage = 0;
    var.loops = 0;
    mission.reset_current();
    last_autoflaps = -1; //force update
    //if(conf.ahrs_agl==agl_auto||conf.ahrs_agl==agl_off) var.power&=~power_agl; // power off AGL sensor
    //if(!conf.ahrs_hgps)//var.cmode&=~cmode_agl; //altitude correction off

    if (apcfg.heli_hYaw > 0)
        var.cmode |= cmode_hyaw;
    if (var.mode == mode_TAKEOFF) {
        onBeforeTakeoff();
    }
    var.ETA = 0;

    var.cams = cams_idle;

    var.ctrb &= ~ctrb_rel;
    var.cmode &= ~cmode_hover;
    var.rwAdj = 0;
    var.ctr_airbrk = 0;
    var.status &= ~status_touch;
    var.cmd_airspeed = apcfg.spd_cruise;
    if (var.cmd_altitude < apcfg.altitude) //climb if too low
        var.cmd_altitude = apcfg.altitude;

    doAutoFlaps = true;

    //zero pilot controls
    if (!(var.status & status_rc)) {
        var.rc_roll = 0;
        var.rc_pitch = 0;
        var.rc_yaw = 0;
        if ((!(var.cmode & cmode_throvr)) && apcfg.type != type_helicopter)
            var.rc_throttle = 0;
    }
}
//==============================================================================
void Shakti::onBeforeTakeoff(void) {}
//==============================================================================
bool Shakti::goWpt(uint n)
{
    var.mode = mode_WPT;
    checkModeChange();
    var.stage = 1;
    //find waypoint
    if (!mission.wp(n)) {
        //dmsg("Waypoint not reachable #%u.\n",n+1);
        dmsg("Mission accomplished\n");
        var.mode = mode_LANDING;
        return false;
    }
    var.wpidx = n;
    const Point &wpNE = var.lla2ne(
        Vect(mission.current.wp->lat, mission.current.wp->lon, mission.current.wp->alt));
    var.cmd_altitude = mission.current.wp->alt;

    set_airspeed(apcfg.spd_cruise);
    mission.current.pi = NULL;

    //some preliminary wpt actions
    if (apcfg.spd_VNO > apcfg.spd_cruise && apcfg.spd_VSI < apcfg.spd_cruise) {
        for (int i = 0;; i++) {
            Mission::_item_action *a = mission.action(i);
            if (!a)
                break;
            switch (a->hdr.option) {
            case Mission::mo_speed: { //fly with spec speed
                _var_float spd = a->speed;
                if (spd > apcfg.spd_VNO)
                    spd = apcfg.spd_VNO;
                else if (spd < apcfg.spd_VSI)
                    spd = apcfg.spd_VSI;
                set_airspeed(spd);
                dmsg("Waypoint action: speed\n");
            } break;
            case Mission::mo_poi: { //look cam to POI
                if (!mission.pi(a->poi)) {
                    dmsg("Warning: POI not found (%u).\n", a->poi);
                    break;
                }
                dmsg("Waypoint action: look\n");
                var.piidx = a->poi;
                lookTo(currentPOI_LLH());
            } break;
            }
        }
    }

    setNE(wpNE);
    var.mtype = mission.current.wp->hdr.option;
    if (var.mtype == mtype_line) {
        if (n == 0)
            var.mtype = mtype_hdg;
        else {
            Mission::_item_wp *cwp = mission.current.wp; //preserve current wp
            Mission::_item_wp *pwp = mission.wp(n - 1);
            mission.current.wp = cwp;
            if (!pwp)
                var.mtype = mtype_hdg;
            else
                var.tgHDG = var.heading(wpNE - var.lla2ne(Vect(pwp->lat, pwp->lon, pwp->alt)));
        }
    }
    dmsg("Going to waypoint #%u.\n", n + 1);
    return true;
}
//==============================================================================
void Shakti::doWaypointActions(void)
{
    for (int i = 0;; i++) {
        Mission::_item_action *a = mission.action(i);
        if (!a)
            break;
        switch (a->hdr.option) {
        case Mission::mo_shot: { //make a cam shot
            dmsg("Waypoint action: shot\n");
            //var.cam_ctrb|=cam_ctrb_shot;
            switch (a->shot.opt) {
            default:
            case 0:
                var.cams = cams_shot;
                break;
            case 1:
                var.cams = cams_dshot;
                var.cam_tperiod = a->shot.dist;
                dshot_ll.fill();
                break;
            case 2:
                var.cams = cams_idle;
                break;
            }
        } break;
        case Mission::mo_loiter: { //visit POI if any
            dmsg("Waypoint action: loiter\n");
            var.wpidx++;                                //next wp after finish
            Mission::_item_wp *wp = mission.current.wp; //save for mode change
            if (mission.current.pi)
                goPi(var.piidx);
            else {
                var.mode = mode_STBY;
                checkModeChange();
                mission.current.loops = 1;
            }
            //override limits
            if (a->loiter.turnR != 0)
                mission.current.turnR = a->loiter.turnR;
            if (a->loiter.loops > 0)
                mission.current.loops = a->loiter.loops;
            if (a->loiter.timeS > 0)
                mission.current.timeS = a->loiter.timeS;
            mission.current.wp = wp; //restore
        } break;
        case Mission::mo_scr: { //execute script
            if (a->scr[0] && strlen(a->scr) < 64) {
                dmsg("Waypoint action: %s\n", a->scr);
                scr_exec(a->scr);
            }
        } break;
        }
    }
}
void Shakti::scr_exec(const char *scr) {}
//==============================================================================
bool Shakti::goPi(uint n)
{
    var.mode = mode_STBY;
    checkModeChange();
    var.stage = 0;
    //find point of interest
    if (!mission.pi(n)) {
        dmsg("Warning: POI not found (%u).\n", n + 1);
        var.piidx = 0;
        return goWpt(var.wpidx);
    }
    var.piidx = n;
    mission.current.turnR = mission.current.pi->turnR;
    mission.current.loops = mission.current.pi->loops;
    mission.current.timeS = mission.current.pi->timeS;
    dmsg("Point of interest #%u.\n", var.piidx + 1);
    return true;
}
//==============================================================================
Vect Shakti::currentPOI_LLH(void)
{
    if (!mission.current.pi)
        return Vect();
    return Vect(Vect(mission.current.pi->lat,
                     mission.current.pi->lon,
                     mission.current.pi->hmsl == 0 ? var.home_pos[2] : mission.current.pi->hmsl));
}
//==============================================================================
void Shakti::calcETA(_var_float dWPT, _var_float spd, bool checkHDG)
{
    uint idt = t - time_eta_s;
    if (idt < 1000)
        return;
    time_eta_s = t;
    if (spd == 0)
        spd = var.gSpeed;
    _var_float dot = (last_dWPT - dWPT);
    last_dWPT = dWPT;
    if (dot > (spd * 2.0)) { //wpt moved
        dot_dWPT = spd;
        var.ETA = 0;
    } else if (var.ETA && dot < 1) { //stopped after move
        dot_dWPT = 0;
        //ETA++;
    } else if (spd < 0.5) { //stopped
        dot_dWPT = 0;
        var.ETA = 0;
    } else if (checkHDG && dot < (spd * 0.7)) { // wrong course
        dot_dWPT = spd;
        var.ETA = dWPT / dot_dWPT;
    } else { //arriving
        dot_dWPT = spd * 0.5 + dot_dWPT * 0.5;
        var.ETA = dWPT / dot_dWPT;
    }
    if (var.ETA > (60 * 60 * 48)) {
        var.ETA = 0;
        dot_dWPT = 0;
    }
}
//==============================================================================
//==============================================================================
void Shakti::control_cam()
{
    //shooting controls
    if ((t - time_shtr_s) > 200)
        var.cam_ctrb &= ~cam_ctrb_shtr;
    switch (var.cams) {
    default:
    case cams_idle:
        break;
    case cams_shot:
        var.cam_ctrb |= cam_ctrb_shtr;
        time_shtr_s = t;
        var.cams = cams_idle;
        dmsg("cam shot\n");
        break;
    case cams_dshot:
        if (var.cam_tperiod) {
            _var_point pLL(var.gps_pos[0], var.gps_pos[1]);
            if (var.distance(dshot_ll, pLL) >= var.cam_tperiod) {
                dshot_ll = pLL;
                var.cam_ctrb |= cam_ctrb_shtr;
                time_shtr_s = t;
                dmsg("cam dshot\n");
            }
        }
        break;
    }

    //gimbal controls
    if (!apcfg.cam_control)
        return;

    if (var.ctrb & ctrb_ers)
        var.power &= ~power_payload;
    if (!(var.power & power_payload))
        var.cam_mode = cam_mode_camoff;
    else if (var.cam_mode == cam_mode_camoff) {
        //cam turned on after was off
        switch (apcfg.cam_default) {
        case cam_default_stab:
            var.cam_mode = cam_mode_stab;
            if (apcfg.cam_control == cam_control_down) {
                var.cambias_theta[0] = var.cambias_theta[2] = var.theta[2];
            }
            break;
        case cam_default_position:
            var.cam_mode = cam_mode_position;
            break;
        }
    }
    if (var.cam_mode != cam_mode_s) {
        switch (var.cam_mode) {
        case cam_mode_stab:
            var.cambias_theta = var.cam_theta;
            break;
        case cam_mode_position:
            var.cambias_theta = Vect();
            break;
        }
        cam_mode_s = var.cam_mode;
    }

    bool doStab = false;
    switch (var.cam_mode) {
    case cam_mode_camoff:
        var.camctr_theta[0] = apcfg.cam_stow.x / 180.0;
        var.camctr_theta[1] = apcfg.cam_stow.y / 180.0;
        var.camctr_theta[2] = apcfg.cam_stow.z / 180.0;
        var.cambias_theta.fill();
        var.cam_theta.fill();
        break;
    case cam_mode_stab:
        //accept roll control from gcu as hdg
        //if(var.camcmd_theta[2]==0 && var.camcmd_theta[0]!=0)var.camcmd_theta[2]=var.camcmd_theta[0];
        var.cambias_theta += var.camcmd_theta * dt * var.limit(1.0 - var.cam_zoom, 0.1, 1);
        var.cambias_theta = var.boundAngle(var.cambias_theta);
        doStab = true;
        break;
    case cam_mode_position:
        //if(var.camcmd_theta[2]==0 && var.camcmd_theta[0]!=0)var.camcmd_theta[2]=var.camcmd_theta[0];
        var.cambias_theta = var.boundAngle(
            var.cambias_theta + var.camcmd_theta * dt * var.limit(1.0 - var.cam_zoom, 0.1, 1));
        var.camctr_theta = var.cambias_theta / 180.0;
        break;
    case cam_mode_speed:
        break;
    case cam_mode_target: {
        //turn camera to llh
        Point ne = var.llh2ne(var.cam_tpos, var.gps_pos);
        //adjust position
        if ((t - time_cam_s) > 100) {
            time_cam_s = t;
            Point adj;
            const _var_float dw = 0.1;
            Point rc((var.camcmd_theta[0] + var.camcmd_theta[2]) / 180.0,
                     var.camcmd_theta[1] / 90.0);
            if (rc[0] > dw)
                adj[1] = rc[0] - dw;
            else if (rc[0] < -dw)
                adj[1] = rc[0] + dw;
            if (rc[1] > dw)
                adj[0] = -rc[1] + dw;
            else if (rc[1] < -dw)
                adj[0] = -rc[1] - dw;
            if (!adj.isNull()) {
                adj = var.rotate(adj * 10.0, -var.heading(ne));
                ne += adj;
                Point ll = var.ne2ll(ne, var.gps_pos);
                var.cam_tpos[0] = ll[0];
                var.cam_tpos[1] = ll[1];
            }
        }
        if (apcfg.cam_control == cam_control_down) {
            var.cambias_theta[2] = var.heading(ne);
            var.cambias_theta[1] = var.heading(var.distance(ne), var.cam_tpos[2] - var.gps_pos[2]);
        } else if (apcfg.cam_control == cam_control_front || apcfg.cam_control == cam_control_roll) {
            ne = var.rotate(ne, var.theta[2]);
            var.cambias_theta[0] = -var.heading(var.gps_pos[2] - var.cam_tpos[2], ne[1]);
            var.cambias_theta[1] = -var.heading(ne[0],
                                                var.distance(var.cam_tpos[2] - var.gps_pos[2],
                                                             ne[1]));
        }
        doStab = true;
    } break;
    }
    switch (apcfg.cam_control) {
    case cam_control_front:
        if (doStab)
            cam_stab_front();
        var.cam_theta[0] = var.cambias_theta[0];
        var.cam_theta[1] = var.cambias_theta[1];
        var.cam_theta[2] = 0;
        var.camctr_theta += Vect(apcfg.cam_align.x, apcfg.cam_align.y, apcfg.cam_align.z) / 180.0;
        break;
    case cam_control_down:
        if (doStab)
            cam_stab_down();
        //var.cam_theta=Quat(var.theta*D2R).conjugate().qmult(var.camctr_theta*D2R).toEuler()*R2D;
        var.cam_theta = Quat(var.theta * D2R).qmult(var.camctr_theta * (180.0 * D2R)).toEuler()
                        * R2D;
        var.cam_theta[0] = 0;
        var.camctr_theta += Vect(apcfg.cam_align.x, apcfg.cam_align.y, apcfg.cam_align.z) / 180.0;
        /*if(var.cam_mode==cam_mode_position){
        var.cam_theta[1]=var.camctr_theta[1]*180.0;
        var.cam_theta[2]=var.camctr_theta[2]*180.0;
      }else{
        var.cam_theta[1]=var.cambias_theta[1];
        var.cam_theta[2]=var.cambias_theta[2];
      }*/
        break;
    case cam_control_roll:
        if (doStab)
            cam_stab_roll();
        var.cam_theta[0] = var.cambias_theta[0];
        var.cam_theta[1] = -90.0;
        var.cam_theta[2] = 0;
        var.camctr_theta += Vect(apcfg.cam_align.x, apcfg.cam_align.y, apcfg.cam_align.z) / 180.0;
        break;
    }
}
//==============================================================================
void Shakti::lookTo(const Vect &llh)
{
    var.cam_tpos = llh;
    if (var.power & power_payload)
        var.cam_mode = cam_mode_target;
}
//==============================================================================
void Shakti::cam_stab_front(void)
{
    Quat qUAV;
    qUAV.fromEuler(Vect(0, var.theta[1] * D2R, 0));
    Quat qGimbal;
    var.cambias_theta[0] = var.limit(var.cambias_theta[0], 80);
    var.cambias_theta[1] = var.limit(var.cambias_theta[1], -100, 10);
    qGimbal.fromEuler(Vect(var.cambias_theta[0] * D2R,
                           var.cambias_theta[1] * D2R,
                           0)); //var.cambias_theta[0]*D2R),Quat::EulOrdXYXs);
    qUAV.conjugate();
    qGimbal.qmult(qUAV);
    Vect euler = qGimbal.toEuler(Quat::EulOrdXYXs) * (R2D);
    if (fabs(euler[2]) > 90) {
        euler[0] = var.boundAngle(180.0 + euler[0]);
        euler[1] = -euler[1];
    }
    if (euler[1] > -15) {
        if (var.cambias_theta[0] > 1)
            var.cambias_theta[0] -= (10.0 + var.cambias_theta[0]) * dt;
        else if (var.cambias_theta[0] < -1)
            var.cambias_theta[0] += (10.0 - var.cambias_theta[0]) * dt;
        else
            var.cambias_theta[0] = 0;
    }
    euler[0] -= var.theta[0]; //roll stab
    var.camctr_theta[0] = euler[0] / 180.0;
    var.camctr_theta[1] = euler[1] / 180.0;
}
//==============================================================================
void Shakti::cam_stab_down(void)
{
    Quat qUAV(var.theta * D2R);
    var.camctr_theta = qUAV.conjugate()
                           .qmult(Vect(0, var.cambias_theta[1] * D2R, (var.cambias_theta[2]) * D2R))
                           .toEuler()
                       * (R2D / 180.0);
}
//==============================================================================
void Shakti::cam_stab_roll(void)
{
    var.camctr_theta[0] = -var.theta[0] / 180.0;
}
//==============================================================================
//==============================================================================
void Shakti::ctr_autoflaps() //_var_float cfgv,bool use_cmd)
{
    _var_float spdMin = apcfg.spd_flapsMin;
    _var_float spdMax = apcfg.spd_flapsMax;
    _var_float v = var.ctr_flaps;
    if (doAutoFlaps && apcfg.flaps_auto > 0 && spdMin > 0
        && spdMax > spdMin) { // && var.airspeed>apcfg.spd_stall
        //autoflaps or manual
        _var_float spdSz = spdMax - spdMin;
        _var_float spd = var.airspeed;
        //if((!use_cmd) || (var.cmode&(cmode_thrcut|cmode_throvr))) spd=var.airspeed;
        //else spd=var.cmd_airspeed;
        if (spd < spdMin)
            spd = spdMin;
        else if (spd > spdMax)
            spd = spdMax;
        _var_float vf = (1.0 - (spd - spdMin) / spdSz) * apcfg.flaps_auto;
        uint vi = vf * 10.0;
        vf = vi * 0.1;
        if (fabs(last_autoflaps - vf) >= 0.1) {
            v = vf;
            last_autoflaps = v;
        }
    }
    //safe speeds
    if (apcfg.spd_VFE > apcfg.spd_flapsMax && var.airspeed > apcfg.spd_VFE)
        v = 0;
    var.ctr_flaps = v;
}
//==============================================================================
//==============================================================================
bool Shakti::ctr_Roll(void)
{
    if (apcfg.Roll.Lo == 0)
        return false;
    _var_float vroll = var.theta[0];
    _var_float croll = var.cmd_theta[0];
    if (apcfg.frm_mode && (apcfg.frm_mode == frm_mode_wingL || apcfg.frm_mode == frm_mode_wingR)) {
        vroll = vroll * (2.0 / 3.0) + var.lrs_croll * (1.0 / 3.0);
        if (apcfg.frm_mode == frm_mode_wingL) {
            croll += var.cmd_vshape;
        } else if (apcfg.frm_mode == frm_mode_wingR) {
            croll -= var.cmd_vshape;
        }
    }
    const _var_float err = var.boundAngle(vroll - croll);
    const _var_float vel = var.gyro[0];
    _var_float Ks = get_gain(var.airspeed, apcfg.Ks_ailerons_low, apcfg.Ks_ailerons_high);
    _var_float Ktas = get_gain_tas();

    _var_float v;
    if (apcfg.dyn_rollReg == dyn_rollReg_PID) {
        reg_Roll.gain[0] = Ktas;
        reg_Roll.gain[1] = Ktas;
        reg_Roll.gain[2] = Ktas;
        v = reg_Roll.step(err, vel, dt);
    } else {
        reg_Roll.gain[0] = Ktas;
        reg_Roll.gain[1] = Ktas;
        reg_Roll.gain[2] = Ktas;
        v = reg_Roll.stepPPI_ail(err, vel, dt);
    }
    if (apcfg.ar_Lp > 0) {
        v = var.limit(v
                          + var.limit(d_gyro[0].step(var.gyro[0], dt) / 100.0 * apcfg.ar_Kp,
                                      apcfg.ar_Lp),
                      apcfg.Roll.Lo);
    }
    Ks *= get_gain_stab(apcfg.stab_ail);
    set_ail(-Ks * v / 100.0);
    return true;
}
//------------------------------------------------------------------------------
bool Shakti::ctr_Pitch(void)
{
    if (apcfg.Pitch.Lo == 0)
        return false;
    _var_float vcmd = var.cmd_theta[1];
    if (apcfg.frm_mode == frm_mode_wingC) {
        //vcmd-=var.ls_ail;
        //vcmd+=var.rs_ail;
        vcmd = vcmd * 3.0 - var.ls_att[2] - var.rs_att[2];
    }
    _var_float err = var.boundAngle(var.theta[1] - vcmd);
    _var_float vel = var.gyro[1];
    _var_float Ks = get_gain(var.airspeed, apcfg.Ks_elevator_low, apcfg.Ks_elevator_high);
    _var_float Ktas = get_gain_tas();

    _var_float v;
    if (apcfg.dyn_rollReg == dyn_pitchReg_PID) {
        reg_Pitch.gain[0] = Ktas;
        reg_Pitch.gain[1] = Ktas;
        reg_Pitch.gain[2] = 1.0;
        v = reg_Pitch.step(err, vel, dt);
    } else {
        vel -= std::abs(R2D * 9.81 * tan(var.cmd_theta[0] * D2R)
                        / (var.cmd_airspeed / get_gain_tas()));

        reg_Pitch.gain[0] = Ktas * cos(var.theta[0] * D2R);
        reg_Pitch.gain[1] = Ktas;
        reg_Pitch.gain[2] = Ktas;
        v = reg_Pitch.stepPPI_elv(err, vel, dt);
    }

    if (apcfg.ap_Lp > 0) {
        v = var.limit(v
                          + var.limit(d_gyro[1].step(var.gyro[1], dt) / 100.0 * apcfg.ap_Kp,
                                      apcfg.ap_Lp),
                      apcfg.Pitch.Lo);
    }
    v = v * Ks / (-100.0);
    v -= var.stab * apcfg.stab_elv;
    set_elv(v);
    return true;
}
//------------------------------------------------------------------------------
bool Shakti::ctr_Yaw(void)
{
    if (apcfg.Yaw.Lo == 0)
        return false;
    switch (apcfg.type) {
    case type_airplane:
    case type_helicopter: {
        _var_float err = var.boundAngle(var.theta[2] - var.rc_yaw * 15.0 - var.cmd_theta[2]);
        _var_float vel = var.gyro[2];
        _var_float v = 0;
        v = reg_Yaw.step(err, vel, dt);
        if (apcfg.ay_Lp > 0) {
            v = var.limit(v
                              + var.limit(d_gyro[2].step(var.gyro[2], dt) / 100.0 * apcfg.ay_Kp,
                                          apcfg.ay_Lp),
                          apcfg.Yaw.Lo);
        }
        v /= 100.0;
        _var_float rud = -v * get_gain_rpm(apcfg.Ks_rudder_rpm);
        if (apcfg.heli_ymin != 0 && rud < apcfg.heli_ymin)
            rud = apcfg.heli_ymin;
        if (apcfg.heli_yrev)
            rud = -rud;
        set_rud(rud);
        set_steering(-v * get_gain(var.gSpeed, apcfg.Ks_steering_low, 0));
    } break;
    case type_blimp: {
        _var_float err = var.boundAngle(var.cmd_course
                                        - ((var.cmode & cmode_hover) ? var.theta[2] : var.course));
        _var_float vel = -var.gyro[2];
        _var_float v = reg_Yaw.step(err, vel, dt) / 100.0;
        set_steering(v); //(var.status&status_glide)?v:0);
    }
        return true;
    case type_boat:
        break;
    }
    return true;
}
//------------------------------------------------------------------------------
bool Shakti::ctr_Thr(void)
{
    if (apcfg.Thr.Lo == 0)
        return false;
    _var_float err = 0;
    switch (apcfg.type) {
    case type_airplane:
        err = var.vspeed - var.cmd_vspeed;
        break;
    case type_helicopter:
        return false;
    case type_blimp:
    case type_boat:
        err = var.airspeed - var.cmd_airspeed;
        break;
    }
    _var_float v = reg_Thr.step(-err, dt) / 100.0;
    if (apcfg.maxThr > 0 && v > apcfg.maxThr) {
        set_thr(apcfg.maxThr, true);
    } else
        set_thr(v, false);
    return true;
}
//------------------------------------------------------------------------------
bool Shakti::ctr_Governor(void)
{
    if (apcfg.Governor.Lo == 0 || (var.error & error_rpm))
        return false;
    //if(apcfg.type==type_helicopter) var.cmd_rpm=var.ctr_brake?apcfg.rpm_idle:apcfg.rpm;
    //else var.cmd_rpm=(_var_float)apcfg.rpm_idle+((_var_float)apcfg.rpm_max-(_var_float)apcfg.rpm_idle)*var.ctr_throttle;
    if (apcfg.type == type_helicopter) {
        if ((var.ctrb & ctrb_gear) && apcfg.Thr.Lo != 0 && apcfg.RpmPitch.Lo != 0) {
            var.cmd_rpm = apcfg.rpm;
            _var_float err = ((_var_float) var.rpm - (_var_float) var.cmd_rpm)
                             / 10000.0; //(_var_float)apcfg.rpm_max;
            _var_float v = reg_RpmPitch.step(err, dt);
            var.ctr_collective = v / 100.0;
            reg_VSpeed.reset(v);
            return true;
        }
        uint target_rpm = var.ctr_brake ? apcfg.rpm_idle : apcfg.rpm;
        if (var.cmd_rpm > target_rpm)
            var.cmd_rpm = target_rpm;
        else {
            uint rpm_step = (_var_float) apcfg.rpm_max * dt * apcfg.to_thrRise;
            if (!rpm_step)
                rpm_step = 1;
            uint new_rpm = var.cmd_rpm + rpm_step;
            if (new_rpm < target_rpm)
                var.cmd_rpm = new_rpm;
            else
                var.cmd_rpm = target_rpm;
        }
    } else
        var.cmd_rpm = (_var_float) apcfg.rpm_idle
                      + ((_var_float) apcfg.rpm_max - (_var_float) apcfg.rpm_idle)
                            * var.ctr_throttle;
    _var_float err = ((_var_float) var.rpm - (_var_float) var.cmd_rpm)
                     / 10000.0; //(_var_float)apcfg.rpm_max;
    _var_float v = reg_Governor.step(-err, dt);
    set_thr(v / 100.0, false);
    reg_Thr.reset(v);
    return true;
}
//------------------------------------------------------------------------------
bool Shakti::ctr_AirbrkApp(void)
{
    if (apcfg.ld_AirbrkApp.Lo == 0)
        return false;
    _var_float err = var.delta;
    _var_float bias = apcfg.ld_Airbrk_bias;
    if (bias < 0 || bias > 1 || (var.error & error_rpm))
        bias = 0;
    _var_float v = var.limit(bias * 100.0 + reg_AirbrkApp.step(err, dt) * (1.0 - bias), 0, 100);
    uint iv = v / 20;
    v = iv * 20;
    v /= 100.0;
    if (fabs(var.ctr_airbrk - v) >= 0.2)
        var.ctr_airbrk = v;
    reg_AirbrkDS.reset((var.ctr_airbrk) * 100.0);
    return true;
}
//------------------------------------------------------------------------------
bool Shakti::ctr_AirbrkDS(_var_float vDS)
{
    if (apcfg.ld_AirbrkDS.Lo == 0)
        return false;
    _var_float err = var.vspeed + vDS;
    _var_float v = var.limit(reg_AirbrkDS.step(err, dt), 0, 100);
    uint iv = v / 10;
    v = iv * 10;
    v /= 100.0;
    if (fabs(var.ctr_airbrk - v) >= 0.1)
        var.ctr_airbrk = v;
    return true;
}
//------------------------------------------------------------------------------
bool Shakti::ctr_Thermal(void)
{
    if (apcfg.Thermal.Lo == 0 || apcfg.type != type_airplane)
        return false;
    _var_float vcenter = var.limit(apcfg.Thermal.Kp * (-var.denergy), apcfg.Thermal.Lp);
    _var_float vseek = var.limit(apcfg.Thermal.Ki * venergy_mean, apcfg.Thermal.Li);
    _var_float v = var.limit(vcenter + vseek, 0, apcfg.Thermal.Lo);
    set_roll(v);
    return true;
}
//------------------------------------------------------------------------------
bool Shakti::ctr_Course(void)
{
    if (apcfg.Course.Lo == 0)
        return false;
    if (isnan(var.cmd_course) || isinf(var.cmd_course))
        var.cmd_course = 0;
    switch (apcfg.type) {
    case type_airplane:
    case type_helicopter: {
        if (apcfg.sf_NoFlyZones != 0 && area.out.dist > 0) {
            if (area.out.isInside) {
                //escape from inside area
                var.cmd_course = var.heading(area.out.intersect);
            } else { // if(apcfg.sf_NoFlyZones==sf_sf_NoFlyZones_on){
                _var_float r = apcfg.turnR * 2.0;
                _var_float dx = var.limit(area.out.dist - apcfg.turnR / 2.0, 0, r);
                _var_float Kr = 1.0 - dx / r;
                _var_float intersect_hdg = var.heading(area.out.intersect);
                _var_float intersect_d = var.boundAngle(intersect_hdg - var.course);
                bool bRight = intersect_d < 0;
                //bRight&=var.boundAngle(var.cmd_course-var.course)>0;
                _var_float turn = (bRight ? 100 : -100.0);
                turn = var.boundAngle(intersect_hdg + turn - var.cmd_course);
                var.cmd_course = var.boundAngle(var.cmd_course + turn * Kr);
            } /*else if(apcfg.sf_NoFlyZones==sf_nofly_field){
          _var_float r=apcfg.turnR*2.0;
          _var_float dx=var.limit(area.out.dist-apcfg.turnR/2.0,0,r);
          _var_float Kr=1.0-dx/r;
          Point wpNE=var.cmd_NE-var.pos_NE;
          Point forceNE=Kr*area.out.sum.norm()+wpNE.norm();
          var.cmd_course=var.heading(forceNE);
        }*/
        }

        _var_float err = var.boundAngle(var.cmd_course - var.course);
        _var_float tLo = apcfg.Course.Lo / 2.0;
        //check already turning behavior
        if (apcfg.type != type_helicopter
            && (var.mode == mode_UAV || var.mode == mode_WPT || var.mode == mode_HOME)) {
            if (err > 90 && var.cmd_theta[0] < (-tLo))
                err -= 360;
            else if (err < (-90) && var.cmd_theta[0] > (tLo))
                err += 360;
        }
        if (apcfg.type == type_helicopter && apcfg.heli_turn != heli_turn_auto && fabs(err) > 90) {
            if (apcfg.heli_turn == heli_turn_left)
                err -= 360;
            else if (apcfg.heli_turn == heli_turn_right)
                err += 360;
        }
        //crs to roll reg PID/PPI
        _var_float vel = -var.gyro[2];
        _var_float v = (apcfg.dyn_courseReg == dyn_courseReg_PID)
                           ? reg_Course.step(err, vel, dt)
                           : reg_Course.stepPPI(err, vel, dt);
        _var_float Ks = get_gain(var.vXY[0] < 0 ? 0 : var.vXY[0],
                                 apcfg.Ks_roll_gs_low == 0 ? 0 : 1.0 / apcfg.Ks_roll_gs_low,
                                 0);
        v /= Ks;
        set_roll(v);
        if (apcfg.heli_Kcrs != 0) {
            _var_float dYaw = var.limit(v * apcfg.heli_Kcrs, -45, 45);
            _var_float vYawr = apcfg.dyn_yaw;
            if (apcfg.heli_Kyawr > 1) {
                _var_float Ksg = get_gain(var.vXY[0] < 0 ? 0 : var.vXY[0], apcfg.heli_Kyawr, 0);
                vYawr *= Ksg;
                dYaw = var.limit(dYaw * Ksg, -45, 45);
            }
            set_yaw(var.boundAngle(var.theta[2] + dYaw), true, vYawr);
        } else
            set_yaw(var.cmd_course);
    } break;
    case type_blimp: {
        _var_float err = var.boundAngle(var.cmd_course - var.course);
        _var_float vel = -var.gyro[2];
        _var_float v = reg_Course.step(err, vel, dt) / 100.0;
        set_rud(v, false);
    } break;
    case type_boat: {
        _var_float err = var.boundAngle(var.cmd_course - var.theta[2]);
        _var_float vel = -var.gyro[2];
        _var_float v = reg_Course.step(err, vel, dt) / 100.0;
        set_rud(v, false);
    } break;
    }
    return true;
}
//------------------------------------------------------------------------------
bool Shakti::ctr_VSpeed(void)
{
    if (apcfg.VSpeed.Lo == 0)
        return false;
    switch (apcfg.type) {
    case type_airplane: {
        _var_float err = var.cmd_vspeed - var.vspeed;
        _var_float vel = var.acc[2] + 10.0;
        if (apcfg.VSpeed.Kp == 0) {
            //alternative vs control
            //err=(var.vspeed*var.vspeed-var.cmd_vspeed*var.cmd_vspeed)/(apcfg.ld_tdAlt/2);
            _var_float err_s = err;
            _var_float Kc = cos(var.theta[0] * D2R) * cos(var.theta[1] * D2R);
            err = (err + 10.0) / Kc + var.acc[2]; //vel;
            vel = 0;                              //var.denergy;
            //var.user1=var.denergy;
            if (var.airspeed == 0)
                err = 0;
            else
                err /= (var.airspeed * var.airspeed);
            _var_float v = reg_VSpeed.step(err, vel, dt);
            //control pitch
            v += var.limit(err_s * apcfg.VSpeed.Kd, apcfg.VSpeed.Ld);
            v *= get_gain(var.airspeed, 0, apcfg.Ks_pitch_high);
            set_pitch(v, false);
            break;
        }
        _var_float v = reg_VSpeed.step(err, vel, dt);
        //control pitch
        v *= get_gain(var.airspeed, 0, apcfg.Ks_pitch_high);
        set_pitch(v, false);
    } break;
    case type_helicopter: {
        if (apcfg.dyn_vspeedMin < 0 && var.cmd_vspeed < apcfg.dyn_vspeedMin)
            var.cmd_vspeed = apcfg.dyn_vspeedMin;
        else if (apcfg.dyn_vspeedMax > 0 && var.cmd_vspeed > apcfg.dyn_vspeedMax)
            var.cmd_vspeed = apcfg.dyn_vspeedMax;

        if (var.cmode & cmode_hover) {
            if (apcfg.heli_vsMinH < 0 && var.cmd_vspeed < apcfg.heli_vsMinH)
                var.cmd_vspeed = apcfg.heli_vsMinH;
            else if (apcfg.heli_vsMaxH > 0 && var.cmd_vspeed > apcfg.heli_vsMaxH)
                var.cmd_vspeed = apcfg.heli_vsMaxH;
        }

        _var_float cmd = var.cmd_vspeed;
        if (apcfg.heli_Ktc > 0 && apcfg.dyn_vspeedMax > 0 && cmd > 0) {
            _var_float Krc = var.limit(1.0 - var.ctr_throttle * apcfg.heli_Ktc, 0, 1);
            _var_float vmax = apcfg.dyn_vspeedMax * Krc;
            if (cmd > vmax)
                cmd = vmax;
        }
        _var_float err = cmd - var.vspeed;
        _var_float vel = var.acc[2] + 9.81;
        _var_float v;
        if ((var.ctrb & ctrb_gear) && apcfg.Thr.Lo != 0 && apcfg.RpmPitch.Lo != 0) {
            v = reg_Thr.step(err, dt);
            set_thr(v / 100, false);
            reg_Governor.reset(v);
        } else {
            v = reg_VSpeed.step(err, vel, dt) / get_gain_rpm(apcfg.Ks_collective_rpm);
            var.ctr_collective = v / 100.0;
            reg_RpmPitch.reset(v);
        }
    } break;
    case type_blimp: {
        _var_float err = var.cmd_vspeed - var.vspeed;
        _var_float vel = var.acc[2]
                         + 9.81; //Vect(var.acc-var.rotate(Vect(0,0,-9.81),var.theta))[2];
        _var_float v = reg_VSpeed.step(err, vel, dt);
        //control pitch
        v *= get_gain(var.airspeed, 0, apcfg.Ks_pitch_high);
        set_pitch(v);
    } break;
    case type_boat:
        break;
    }
    return true;
}
//------------------------------------------------------------------------------
bool Shakti::ctr_Altitude(void)
{
    if (apcfg.Altitude.Lo == 0)
        return false;
    _var_float err = var.cmd_altitude - var.altitude; //use corrected altitude for controls
    _var_float v = reg_Altitude.step(err);
    set_vspeed(v);
    return true;
}
//------------------------------------------------------------------------------
bool Shakti::ctr_Taxi(void)
{
    if (apcfg.Taxi.Lo == 0)
        return false;
    _var_float err = var.rwDelta;
    _var_float vel = var.rwDV;
    _var_float turn90 = 0;
    bool bTurn90 = false;
    if (var.mode == mode_TAXI) {
        _var_float sdist = apcfg.taxi_turnR > 0 ? apcfg.taxi_turnR : 3;
        if (var.rwDelta > sdist) {
            bTurn90 = true;
            turn90 = var.boundAngle(var.tgHDG - 90);
        } else if (var.rwDelta < -sdist) {
            bTurn90 = true;
            turn90 = var.boundAngle(var.tgHDG + 90);
        }
    }
    if (apcfg.Yaw.Lo != 0) {
        _var_float v = bTurn90 ? turn90 : var.boundAngle(var.tgHDG - reg_Taxi.step(err, vel, dt));
        set_yaw(v, false);
        var.cmd_course = v;
    } else {
        _var_float v = -reg_Taxi.stepPPI(err, vel, dt) / 100.0;
        v += var.limit(-var.gyro[2] * apcfg.Yaw.Kp + var.rc_yaw * 15.0, apcfg.Yaw.Lp) / 100.0;
        set_rud(v * get_gain(var.airspeed, apcfg.Ks_rudder_low, apcfg.Ks_rudder_high), false);
        set_steering(v * get_gain(var.gSpeed, apcfg.Ks_steering_low, 0));
        var.cmd_course = var.tgHDG + v * 25.0; //display only
    }
    return true;
}
//------------------------------------------------------------------------------
bool Shakti::ctr_Brakes(void)
{
    if (apcfg.Brakes.Lo == 0 || apcfg.taxi_gSpeed <= 0)
        return false;
    if (apcfg.type != type_airplane)
        return false;
    if (var.ctr_brake == 1.0) {
        reg_Brakes.reset();
        last_brake = 0;
        return true;
    }
    if (var.ctr_brake < 0.005 && var.ctr_brake != last_brake) {
        reg_Brakes.reset();
        last_brake = var.ctr_brake;
        var.ctr_brake = 1;
        return true;
    }
    if (var.ctr_brake == 0) {
        print_status_float(idx_ctr_brake, &ctr_brake_s);
    }
    _var_float err = var.gSpeed - var.cmd_airspeed;
    var.ctr_brake = var.limit(reg_Brakes.step(err, dt) / 100.0, 0.01, 0.98);
    last_brake = var.ctr_brake;
    ctr_brake_s = var.ctr_brake;
    return true;
}
//------------------------------------------------------------------------------
bool Shakti::ctr_Line(void)
{
    if (apcfg.Line.Lo == 0)
        return false;
    _var_float err = var.rwDelta;
    _var_float vel = var.rwDV;
    _var_float adj = var.limit(var.boundAngle(((apcfg.Line.Ki == 0) ? reg_Line.stepPPI(err, vel, dt)
                                                                    : reg_Line.step(err, vel, dt))),
                               90);
    //_var_float cd=var.boundAngle(var.course-var.tgHDG);
    _var_float sdist = apcfg.turnR;
    if (var.rwDelta > sdist)
        adj = 90;
    else if (var.rwDelta < -sdist)
        adj = -90;
    var.cmd_course = var.boundAngle(var.tgHDG - adj);
    return true;
}
//------------------------------------------------------------------------------
bool Shakti::ctr_Path(void)
{
    if (apcfg.Path.Lo == 0)
        return false;
    _var_float err = var.rwDelta;
    _var_float vel = var.rwDV;
    _var_float reg = reg_Path.step(err, vel, dt);
    if (apcfg.Path.Li == 0 && apcfg.Path.Ki != 0 && var.turnR != 0)
        reg = var.limit(reg - apcfg.Path.Ki * var.gSpeed * var.gSpeed / var.turnR, apcfg.Path.Lo);
    _var_float adj = var.limit(reg, 90);
    //_var_float cd=var.boundAngle(var.course-var.tgHDG);
    //_var_float sdist=apcfg.turnR;
    //if((var.rwDelta>0 && cd>=90) || var.rwDelta>sdist) adj=90;
    //else if((var.rwDelta<0 && cd<-90) || var.rwDelta<-sdist) adj=-90;
    var.cmd_course = var.boundAngle(var.tgHDG - adj);
    return true;
}
//------------------------------------------------------------------------------
bool Shakti::ctr_Airspeed(void)
{
    if (apcfg.Airspeed.Lo == 0)
        return false;
    smooth_cmd_airspeed();
    _var_float cmd = cmd_airspeed_dyn / 100.0;
    switch (apcfg.type) {
    case type_airplane: {
        //protect stall speed from roll stall(correct)=stall*sqrt(1/cos(cmd_roll))
        if (apcfg.spd_stall > 0 && apcfg.spd_stall < apcfg.spd_cruise) {
            _var_float vmin = apcfg.spd_stall
                              * sqrt(1.0 / cos(var.limit(var.cmd_theta[0], -80, 80) * D2R));
            if (cmd < vmin)
                cmd = vmin;
        }
        _var_float err = var.airspeed - cmd;
        _var_float v = reg_Airspeed.step(err, -var.vcas, dt);
        if (apcfg.dyn_pitchDiff > 0 && apcfg.dyn_pitchDiff < 1.0) {
            _var_float vmin = -apcfg.dyn_pitchDiff * apcfg.Airspeed.Lo;
            if (v < vmin)
                v = vmin;
            vmin = -apcfg.dyn_pitchDiff * apcfg.Airspeed.Li;
            if (reg_Airspeed.sum < vmin)
                reg_Airspeed.sum = vmin;
        }
        reg_Airspeed.out = v;
        set_pitch(v);
    } break;
    case type_helicopter: {
        _var_float err = var.cmd_airspeed >= apcfg.heli_aspd ? var.airspeed : var.vXY[0];
        err -= cmd;
        _var_float v = reg_Airspeed.step(err, -var.vcas, dt);
        set_pitch(v);
    } break;
    case type_blimp: {
        _var_float err = var.cmd_vspeed - var.vspeed;
        _var_float v = reg_Airspeed.step(err, 0, dt);
        var.ctr_collective = v / 100.0;
    } break;
    case type_boat:
        break;
    }
    return true;
}
//------------------------------------------------------------------------------
bool Shakti::ctr_Slip(void)
{
    if (apcfg.Slip.Lo == 0)
        return false;
    _var_float err = var.slip - var.cmd_slip;
    //_var_float vel=(apcfg.frm_mode)?var.gyro[0]:var.gyro[2];
    _var_float vel = var.gyro[2];

    _var_float Ktas = get_gain_tas();

    _var_float v;
    if (apcfg.dyn_rollReg == dyn_slipReg_PID) {
        reg_Slip.gain[0] = Ktas;
        reg_Slip.gain[1] = Ktas;
        reg_Slip.gain[2] = Ktas;
        v = reg_Slip.step(err, -vel, dt) / 100.0 + var.rc_yaw;
    } else {
        reg_Slip.gain[0] = Ktas;
        reg_Slip.gain[1] = Ktas;
        reg_Slip.gain[2] = Ktas;
        vel -= var.limit(err * apcfg.Slip.Kp, apcfg.Slip.Lp);
        if (var.cmd_airspeed > 0)
            vel -= R2D * 9.81 * tan(var.cmd_theta[0] * D2R)
                   / (var.cmd_airspeed / get_gain_tas()); //*cos(var.theta[0]*D2R)
        v = -reg_Slip.stepPPI(0, vel, dt) / 100.0 + var.rc_yaw;
    }

    _var_float Ks = get_gain(var.airspeed, apcfg.Ks_rudder_low, apcfg.Ks_rudder_high);
    set_rud(v * Ks);
    return true;
}
//------------------------------------------------------------------------------
bool Shakti::ctr_HoverX(void)
{
    if (apcfg.HoverX.Lo == 0)
        return false;
    _var_float err = -var.dXY[0];
    _var_float vel = var.vXY[0] + var.rc_pitch * 10.0;
    set_pitch(+reg_HoverX.step(err, vel, dt), true, apcfg.dyn_pitch * 2.0);
    return true;
}
//------------------------------------------------------------------------------
bool Shakti::ctr_HoverY(void)
{
    if (apcfg.HoverY.Lo == 0)
        return false;
    _var_float err = -var.dXY[1];
    _var_float vel = var.vXY[1] - var.rc_roll * 10.0;
    set_roll(-reg_HoverY.step(err, vel, dt), true, apcfg.dyn_roll * 2.0);
    if ((var.cmode & cmode_hyaw) && apcfg.heli_hYaw > 0
        && fabs(var.cmd_theta[0]) > apcfg.heli_hYaw) {
        if (var.cmd_theta[0] > 0)
            var.cmd_course += 5.0 * dt;
        else
            var.cmd_course -= 5.0 * dt;
        var.cmd_course = var.boundAngle(var.cmd_course);
    }
    return true;
}
//==============================================================================
//==============================================================================
_var_float Shakti::get_gain(_var_float speed, _var_float Ks_low, _var_float Ks_high)
{
    _var_float cs = apcfg.spd_cruise;
    // square relation to speed
    if (speed <= cs) {
        _var_float ls = apcfg.Ks_speed_low;
        if (Ks_low <= 1.0 || ls >= cs)
            return 1.0;
        if (speed <= ls)
            return Ks_low;
        _var_float x = 1.0 - (speed - ls) / (cs - ls);
        return x * x * (Ks_low - 1.0) + 1.0;
    }
    _var_float hs = apcfg.Ks_speed_high;
    if (Ks_high == 0 || Ks_high >= 1.0 || hs <= cs)
        return 1.0;
    if (speed >= hs)
        return Ks_high;
    _var_float x = (speed - cs) / (hs - cs);
    return 1.0 / (x * x * ((1.0 - Ks_high) / Ks_high) + 1.0);
}
_var_float Shakti::get_gain_rpm(_var_float Ks_low)
{
    if (var.error & error_rpm || var.rpm >= apcfg.rpm)
        return 1;
    _var_float rpm = var.rpm;
    _var_float cs = apcfg.rpm;
    // square relation to speed
    _var_float ls = apcfg.rpm_idle;
    if (Ks_low <= 1.0 || ls >= cs)
        return 1;
    if (rpm <= ls)
        return Ks_low;
    _var_float x = 1.0 - (rpm - ls) / (cs - ls);
    return x * x * (Ks_low - 1.0) + 1.0;
}
_var_float Shakti::get_gain_tas()
{
    if (!apcfg.Ks_tas_stab)
        return 1.0;
    if (apcfg.spd_cruise == 0)
        return 1.0;

    _var_float speed = var.airspeed;
    if (var.error & error_cas)
        speed = apcfg.spd_cruise;

    _var_float alt = var.gps_pos[2];
    if (alt < 0)
        alt = 0;
    else if (alt > 35000.0)
        alt = 35000.0;
    _var_float tas = sqrtf(powf(1.0 - 0.00002257 * alt, 4.255));
    if (isinf(tas) || isnan(tas))
        tas = 0.001;

    _var_float k = tas * apcfg.spd_cruise / speed;
    if (k > 2.0)
        k = 2.0;

    return k;
}
//==============================================================================
_var_float Shakti::get_gain_stab(_var_float Ks_slw)
{
    if (var.mode == mode_TAKEOFF || var.mode == mode_TAXI
        || (var.mode == mode_LANDING && var.stage > 2))
        return 1.0;
    if (Ks_slw == 0 || Ks_slw > 1 || Ks_slw < -1)
        return 1.0;
    _var_float stab = var.limit(var.stab, 0, 1);
    return 1.0 - (1.0 - Ks_slw) / 2.0 * (1.0 - cos((2.0 * M_PI) / (1.0 + stab)));
}
//==============================================================================
//==============================================================================
void Shakti::set_ail(_var_float v)
{
    var.ctr_ailerons = var.limit(v, 1.0);
}
//-----------------------------
void Shakti::set_elv(_var_float v, bool doMix)
{
    if (doMix)
        v += var.limit(get_gain(var.airspeed, 0, apcfg.Ks_elevator_high) * fabs(var.cmd_theta[0])
                           * apcfg.mix_elv_Kp,
                       apcfg.mix_elv_Lo)
             / 100.0; //mix to roll
    var.ctr_elevator = var.limit(v, 1.0);
}
//-----------------------------
void Shakti::set_thr(_var_float v, bool doResetPID)
{
    v = var.limit(v, 0, 1);
    if (var.cmode & cmode_thrcut) {
        v = 0;
        doResetPID = true;
    } else if (var.cmode & cmode_throvr) {
        v = var.rc_throttle;
        doResetPID = true;
    }
    var.ctr_throttle = v;
    if (doResetPID) {
        reg_Thr.reset(v * 100.0);
        reg_Governor.reset(v * 100.0);
    }
}
//-----------------------------
void Shakti::set_rud(_var_float v, bool doMix)
{
    if (doMix) {
        if (apcfg.type == type_helicopter) {
            _var_float Ks = get_gain_rpm(apcfg.Ks_rudder_rpm);
            v += var.limit(Ks * fabs(var.ctr_collective) * apcfg.mix_crud_Kp, apcfg.mix_crud_Lo)
                 / 100.0; //mix to collective
        } else {
            _var_float Ks = get_gain(var.airspeed, 0, apcfg.Ks_rudder_high);
            v += var.limit(Ks * var.cmd_theta[0] * apcfg.mix_rud_Kp, apcfg.mix_rud_Lo)
                 / 100.0; //mix to roll
            v += var.limit(Ks * var.gyro[0] * apcfg.mix_grud_Kp, apcfg.mix_grud_Lo)
                 / 100.0; //mix to roll gyro
        }
    }
    var.ctr_rudder = var.limit(v, 1.0);
}
//-----------------------------
void Shakti::set_steering(_var_float v)
{
    var.ctr_steering = var.limit(v, 1.0);
}
//-----------------------------
void Shakti::set_roll(_var_float v, bool smooth, _var_float spd)
{
    if (apcfg.type == type_airplane
        || (apcfg.type == type_helicopter && var.mode != mode_STBY && var.mode != mode_UAV))
        v += var.rc_roll * 20.0; //correct roll from joystick
    if (!smooth)
        spd = 0;
    else if (spd == 0)
        spd = apcfg.dyn_roll;
    ctr_smooth(&(var.cmd_theta[0]), v, spd);
}
//-----------------------------
void Shakti::set_pitch(_var_float v, bool smooth, _var_float spd)
{
    if (apcfg.type == type_airplane
        || (apcfg.type == type_helicopter && var.mode != mode_STBY && var.mode != mode_UAV))
        v += var.rc_pitch * 20.0; //always correct pitch from joystick
    if (!smooth)
        spd = 0;
    else if (spd == 0)
        spd = apcfg.dyn_pitch;
    ctr_smooth(&(var.cmd_theta[1]), v, spd);
    var.cmd_theta[1] = var.limit(var.cmd_theta[1], -45, 45);
}
//-----------------------------
void Shakti::set_yaw(_var_float v, bool smooth, _var_float spd)
{
    if (!smooth)
        spd = 0;
    else if (spd == 0)
        spd = apcfg.dyn_yaw;
    var.cmd_theta[2] = var.smoothAngle(v, var.cmd_theta[2], spd * dt);
}
//-----------------------------
void Shakti::set_vspeed(_var_float v, bool smooth, _var_float spd)
{
    if (!smooth)
        spd = 0;
    else if (spd == 0)
        spd = apcfg.dyn_vspeed;
    ctr_smooth(&(var.cmd_vspeed), v, var.cmd_vspeed > v ? spd : (spd * 2.0));
}
//-----------------------------
void Shakti::set_airspeed(_var_float v)
{
    var.cmd_airspeed = v;
    if (var.cmd_airspeed < apcfg.spd_stall)
        var.cmd_airspeed = apcfg.spd_stall;
    else if (apcfg.spd_VSI > apcfg.spd_stall && var.ctr_flaps == 0
             && var.cmd_airspeed < apcfg.spd_VSI)
        var.cmd_airspeed = apcfg.spd_VSI;
    else if (apcfg.spd_VNE > apcfg.spd_stall && var.cmd_airspeed > apcfg.spd_VNE)
        var.cmd_airspeed = apcfg.spd_VNE;
}
//-----------------------------

//==============================================================================
void Shakti::ctr_smooth(_var_float *v, _var_float vSet, _var_float speed)
{
    if (isinf(vSet) || isnan(vSet))
        vSet = 0;
    _var_float delta = vSet - *v;
    _var_float rstep = speed * dt;
    if (delta == 0.0)
        return;
    if (rstep == 0.0) {
        *v = vSet;
    } else if (delta > 0) {
        if (rstep > delta)
            *v = vSet;
        else
            *v += rstep;
    } else {
        if (rstep > (-delta))
            *v = vSet;
        else
            *v -= rstep;
    }
}
//==============================================================================
void Shakti::smooth_cmd_airspeed()
{
    _var_float v = var.cmd_airspeed * 100.0;
    if (apcfg.dyn_speed > 0) {
        if (cmd_airspeed_dyn > v)
            ctr_smooth(&cmd_airspeed_dyn, v, apcfg.dyn_speed * (100.0 / 2.0));
        else
            cmd_airspeed_dyn = v;
    } else
        cmd_airspeed_dyn = v;
}
//==============================================================================
//==============================================================================
void Shakti::print_status_flag(uint8_t var_idx, _var_flag value_prev)
{
    uint type;
    void *value_ptr;
    if (!var.get_ptr(var_idx, &value_ptr, &type))
        return;
    _var_flag v = *((_var_flag *) value_ptr);
    _var_flag change = value_prev ^ v;
    if (!change)
        return;
    _var_flag smask = 1;
    while (smask) {
        if (change & smask) {
            const char *name;
            const char *descr;
            if (!var.get_text_names(var_idx | (smask << 8), &name, &descr))
                break;
            const char *str = descr;
            const char *pos_slash = rindex(str, '/');
            const char *pos_space = rindex(str, ' ');
            if ((pos_slash && pos_space) && (pos_slash > pos_space)) {
                char s[64] = "";
                strcat(s, str);
                char *s_slash = rindex(s, '/');
                char *s_space = rindex(s, ' ');
                *s_slash = '\0';
                *s_space = '\0';
                dmsg("%s %s\n", s, (v & smask) ? (s_space + 1) : (s_slash + 1));
                s[0] = '\0';
            }
        }
        smask <<= 1;
    }
}
//==============================================================================
void Shakti::print_status_enum(uint8_t var_idx, _var_enum value_prev)
{
    uint type;
    void *value_ptr;
    if (!var.get_ptr(var_idx, &value_ptr, &type))
        return;
    _var_enum v = *((_var_enum *) value_ptr);
    _var_enum change = value_prev != v;
    if (!change)
        return;
    const char *var_name;
    const char *var_descr;
    const char *name;
    const char *descr;
    if (!var.get_text_names(var_idx | 0xFF00, &var_name, &var_descr))
        return;
    if (!var.get_text_names(var_idx | (v << 8), &name, &descr))
        return;
    dmsg("%s: %s\n", var_descr, descr);
}
//==============================================================================
void Shakti::print_status_float(uint8_t var_idx, _var_float *value_prev)
{
    uint type;
    void *value_ptr;
    if (!var.get_ptr(var_idx, &value_ptr, &type))
        return;
    _var_float v = *((_var_float *) value_ptr);
    _var_float change = *value_prev != v;
    if (!change)
        return;
    *value_prev = v;
    const char *name;
    const char *descr;
    if (!var.get_text_names(var_idx, &name, &descr))
        return;
    const char *str = descr;
    char *pos_slash = (char *) rindex(str, '[');
    if (pos_slash) {
        char s[64] = "";
        strcat(s, str);
        pos_slash = rindex(s, '[') - 1;
        *pos_slash = '\0';
        dmsg("%s %s\n", s, (v > 0.0) ? "on" : "off");
    }
}
//==============================================================================
//==============================================================================
void Shakti::check_config(void)
{
    if (apcfg.wpt_snap == 0)
        apcfg.wpt_snap = 15;
    if (apcfg.turnR == 0)
        apcfg.turnR = 300;

    switch (apcfg.type) {
    case type_airplane:
        if (apcfg.to_liftAlt == 0)
            apcfg.to_liftAlt = 10;
        if (apcfg.to_liftPitch == 0)
            apcfg.to_liftPitch = 10;
        if (apcfg.spd_stall == 0)
            apcfg.spd_stall = 15;
        //if(apcfg.spd_VSI==0)apcfg.spd_VSI=16;
        //if(apcfg.spd_VFE==0)apcfg.spd_VFE=17;
        //if(apcfg.spd_VNO==0)apcfg.spd_VNO=(apcfg.spd_cruise==0)?20:apcfg.spd_cruise;
        if (apcfg.spd_VY <= apcfg.spd_stall)
            apcfg.spd_VY = apcfg.spd_stall + 1;
        if (apcfg.spd_cruise == 0)
            apcfg.spd_cruise = apcfg.spd_stall + 3;
        break;
    case type_helicopter:
        if (apcfg.spd_cruise == 0)
            apcfg.spd_cruise = 5;
        break;
    case type_blimp:
        if (apcfg.spd_cruise == 0)
            apcfg.spd_cruise = 5;
        break;
    case type_boat:
        if (apcfg.spd_cruise == 0)
            apcfg.spd_cruise = 5;
        break;
    }
}
//==============================================================================
//==============================================================================
_var_float REG_PID::step(_var_float err, _var_float vel, _var_float dt)
{
    if (isinf(err) || isnan(err))
        err = 0;
    if (isinf(vel) || isnan(vel))
        vel = 0;
    out = var.limit(cfg->Kp * gain[0] * err, cfg->Lp) + var.limit(cfg->Kd * gain[1] * vel, cfg->Ld)
          + sum;
    if (cfg->Ki == 0)
        sum = 0;
    else
        sum = var.limit(sum + err * cfg->Ki * gain[2] * dt, cfg->Li);
    out = var.limit(out, cfg->Lo);
    return out;
}
_var_float REG_PID::stepPPI(_var_float err, _var_float vel, _var_float dt)
{
    if (isinf(err) || isnan(err))
        err = 0;
    if (isinf(vel) || isnan(vel))
        vel = 0;
    err = err * cfg->Kp * gain[0];
    if (cfg->Lp != 255)
        err = var.limit(err, cfg->Lp);
    vel += err;
    out = var.limit(cfg->Kd * gain[1] * vel, cfg->Ld) + sum;
    if (cfg->Ki == 0)
        sum = 0;
    else
        sum = var.limit(sum + vel * cfg->Ki * gain[2] * dt, cfg->Li);
    out = var.limit(out, cfg->Lo);
    return out;
}
_var_float REG_PID::stepPPI_elv(_var_float err, _var_float vel, _var_float dt)
{
    //limit error for integral
    if (isinf(err) || isnan(err))
        err = 0;
    if (isinf(vel) || isnan(vel))
        vel = 0;
    err = err * cfg->Kp * gain[0];
    if (cfg->Lp != 255)
        err = var.limit(err, cfg->Lp * gain[0]);
    vel += err;
    if (cfg->Lp != 255)
        vel = var.limit(vel, cfg->Lp * gain[0]);
    out = var.limit(cfg->Kd * gain[1] * vel, cfg->Ld) + sum;
    if (cfg->Ki == 0)
        sum = 0;
    else
        sum = var.limit(sum + vel * cfg->Ki * gain[2] * dt, cfg->Li);
    out = var.limit(out, cfg->Lo);
    return out;
}
_var_float REG_PID::stepPPI_ail(_var_float err, _var_float vel, _var_float dt)
{
    //limit error for integral
    if (isinf(err) || isnan(err))
        err = 0;
    if (isinf(vel) || isnan(vel))
        vel = 0;
    err = err * cfg->Kp * gain[0];
    if (cfg->Lp != 255)
        err = var.limit(err, cfg->Lp);
    vel += err;
    if (cfg->Lp != 255)
        vel = var.limit(vel, cfg->Lp);
    out = var.limit(cfg->Kd * gain[1] * vel, cfg->Ld) + sum;
    if (cfg->Ki == 0)
        sum = 0;
    else
        sum = var.limit(sum + vel * cfg->Ki * gain[2] * dt, cfg->Li);
    out = var.limit(out, cfg->Lo);
    return out;
}
_var_float REG_PI::step(_var_float err, _var_float dt)
{
    if (isinf(err) || isnan(err))
        err = 0;
    out = var.limit(cfg->Kp * err, cfg->Lp) + sum;
    if (cfg->Ki == 0)
        sum = 0;
    else
        sum = var.limit(sum + err * cfg->Ki * dt, cfg->Li);
    out = var.limit(out, cfg->Lo);
    return out;
}
_var_float REG_TPI::step(_var_float err, _var_float dt)
{
    if (isinf(err) || isnan(err))
        err = 0;
    if (cfg->Ki == 0)
        sum = 0;
    else
        sum = var.limit(sum + var.limit(err * cfg->Ki * dt, cfg->Li), 100.0 - cfg->Lo, 100.0);
    out = var.limit(sum + var.limit(cfg->Kp * err, cfg->Lp), 100.0 - cfg->Lo, 100.0);
    return out;
}
_var_float REG_P::step(_var_float err)
{
    if (isinf(err) || isnan(err))
        err = 0;
    return var.limit(cfg->Kp * err, cfg->Lo);
}
_var_float REG_PPI::step(_var_float err, _var_float vel, _var_float dt)
{
    if (isinf(err) || isnan(err))
        err = 0;
    if (isinf(vel) || isnan(vel))
        vel = 0;
    err = err * cfg->Kpp;
    if (cfg->Lpp != 255)
        err = var.limit(err, cfg->Lpp);
    vel += err;
    out = var.limit(cfg->Kp * vel, cfg->Lp) + sum;
    if (cfg->Ki == 0)
        sum = 0;
    else
        sum = var.limit(sum + vel * cfg->Ki * dt, cfg->Li);
    out = var.limit(out, cfg->Lo);
    return out;
}
void REG_PID::reset(const _var_float v)
{
    if (cfg->Ki == 0)
        sum = 0;
    else
        sum = var.limit(v, cfg->Li);
    out = v;
}
void REG_PI::reset(const _var_float v)
{
    if (cfg->Ki == 0)
        sum = 0;
    else
        sum = var.limit(v, cfg->Li);
    out = v;
}
void REG_TPI::reset(const _var_float v)
{
    if (cfg->Ki == 0)
        sum = 0;
    else
        sum = var.limit(v, 100.0 - cfg->Lo, 100.0);
    out = v;
}
void REG_PPI::reset(const _var_float v)
{
    if (cfg->Ki == 0)
        sum = 0;
    else
        sum = var.limit(v, cfg->Li);
    out = v;
}
//==============================================================================
