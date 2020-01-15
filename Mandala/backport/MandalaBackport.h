#pragma once

#include <Mandala/tree/MandalaMetaBase.h>
#include <MandalaMetaTree.h>

namespace mandala {
namespace backport {

enum fmt_id_t {
    f4,
    f2,
    s1,
    s10,
    s01,
    s001,
    u2,
    u4,
    u1,
    u01,
    u001,
    bit
};

struct item_t
{
    const meta_t &meta;
    const char *alias;
    const fmt_id_t fmt_sd;
    const fmt_id_t fmt_hd;
};

#define MBPORT(a, b, c, d) \
    { \
        a, #b, c, d \
    }

static constexpr const meta_t &meta_void = cmd::env::dict::search::meta;

static constexpr const item_t items[] = {
    MBPORT(state::nav::att::roll::meta, roll, f2, f2),
    MBPORT(state::nav::att::pitch::meta, pitch, f2, f2),
    MBPORT(state::nav::att::yaw::meta, yaw, f2, f2),

    MBPORT(state::nav::pos::ax::meta, Ax, f2, s1),
    MBPORT(state::nav::pos::ay::meta, Ay, f2, s1),
    MBPORT(state::nav::pos::az::meta, Az, f2, s1),

    MBPORT(state::nav::att::p::meta, p, f2, s10),
    MBPORT(state::nav::att::q::meta, q, f2, s10),
    MBPORT(state::nav::att::r::meta, r, f2, s10),

    MBPORT(sns::nav::mag::x::meta, Hx, f2, s001),
    MBPORT(sns::nav::mag::y::meta, Hy, f2, s001),
    MBPORT(sns::nav::mag::z::meta, Hz, f2, s001),

    MBPORT(state::nav::air::altitude::meta, altitude, f4, f4),
    MBPORT(state::nav::air::airspeed::meta, airspeed, f2, f2),
    MBPORT(state::nav::air::vspeed::meta, vspeed, f2, f2),
    MBPORT(state::nav::pos::course::meta, course, f2, f2),

    MBPORT(state::nav::ctr::rpm::meta, rpm, u2, u2),
    MBPORT(state::nav::pos::agl::meta, agl, f2, f2),
    MBPORT(state::nav::air::slip::meta, slip, f2, f2),
    MBPORT(state::nav::air::aoa::meta, attack, f2, f2),
    MBPORT(state::nav::est::venergy::meta, venergy, f2, f2),
    MBPORT(state::nav::est::ld::meta, ldratio, f2, f2),
    MBPORT(state::nav::air::buo::meta, buoyancy, f2, f2),

    MBPORT(ctr::nav::stab::ail::meta, ctr_ailerons, f2, s001),
    MBPORT(ctr::nav::stab::elv::meta, ctr_elevator, f2, s001),
    MBPORT(ctr::nav::eng::thr::meta, ctr_throttle, f2, u001),
    MBPORT(ctr::nav::stab::rud::meta, ctr_rudder, f2, s001),
    MBPORT(ctr::nav::stab::col::meta, ctr_collective, f2, s001),

    MBPORT(ctr::nav::str::str::meta, ctr_steering, f2, s001),
    MBPORT(ctr::nav::wing::airbrk::meta, ctr_airbrk, f2, u001),
    MBPORT(ctr::nav::wing::flaps::meta, ctr_flaps, u001, u001),
    MBPORT(ctr::nav::str::brake::meta, ctr_brake, f2, u001),
    MBPORT(ctr::nav::eng::choke::meta, ctr_mixture, f2, u001),
    MBPORT(ctr::nav::eng::tune::meta, ctr_engine, f2, u001),
    MBPORT(ctr::nav::wing::sweep::meta, ctr_sweep, f2, s001),
    MBPORT(ctr::nav::wing::buo::meta, ctr_buoyancy, f2, s001),

    MBPORT(ctr::env::ers::launch::meta, ctrb_ers, bit, bit),
    MBPORT(ctr::env::ers::rel::meta, ctrb_rel, bit, bit),
    MBPORT(ctr::env::door::drop::meta, ctrb_drp, bit, bit),
    MBPORT(ctr::env::sw::fpump::meta, ctrb_pump, bit, bit),
    MBPORT(ctr::env::sw::starter::meta, ctrb_starter, bit, bit),
    MBPORT(ctr::env::sw::horn::meta, ctrb_horn, bit, bit),
    MBPORT(ctr::nav::eng::rev::meta, ctrb_rev, bit, bit),
    MBPORT(ctr::nav::str::gear::meta, ctrb_gear, bit, bit),

    MBPORT(meta_void, ctr_brakeL, f2, u001),
    MBPORT(meta_void, ctr_brakeR, f2, u001),

    MBPORT(cmd::nav::reg::roll::meta, cmd_roll, f2, f2),
    MBPORT(cmd::nav::reg::pitch::meta, cmd_pitch, f2, f2),
    MBPORT(cmd::nav::reg::yaw::meta, cmd_yaw, f2, f2),
    MBPORT(cmd::nav::reg::n::meta, cmd_north, f2, f2),
    MBPORT(cmd::nav::reg::e::meta, cmd_east, f2, f2),
    MBPORT(cmd::nav::reg::course::meta, cmd_course, f2, f2),
    MBPORT(cmd::nav::reg::rpm::meta, cmd_rpm, u2, u2),
    MBPORT(cmd::nav::reg::altitude::meta, cmd_altitude, f2, f2),
    MBPORT(cmd::nav::reg::airspeed::meta, cmd_airspeed, f2, f2),
    MBPORT(cmd::nav::reg::vspeed::meta, cmd_vspeed, f2, s01),
    MBPORT(cmd::nav::reg::slip::meta, cmd_slip, f2, s1),

    MBPORT(state::nav::pos::lat::meta, gps_lat, f4, f4),
    MBPORT(state::nav::pos::lon::meta, gps_lon, f4, f4),
    MBPORT(state::nav::pos::hmsl::meta, gps_hmsl, f4, f4),
    MBPORT(state::nav::pos::vn::meta, gps_Vnorth, f2, f2),
    MBPORT(state::nav::pos::ve::meta, gps_Veast, f2, f2),
    MBPORT(state::nav::pos::vd::meta, gps_Vdown, f2, f2),
    MBPORT(state::env::sys::time::meta, gps_time, u4, u4),

    MBPORT(state::env::sys::energy::meta, fuel, f4, f2),
    MBPORT(meta_void, frate, f4, f2),
    MBPORT(sns::env::com::rss::meta, rss, u001, u001), //convert to 0..1

    MBPORT(sns::env::pwr::vsys::meta, Ve, f2, f2),
    MBPORT(sns::env::pwr::vsrv::meta, Vs, f2, f2),
    MBPORT(sns::env::pwr::vpld::meta, Vp, f2, f2),
    MBPORT(sns::env::eng::voltage::meta, Vm, f2, f2),
    MBPORT(sns::env::pwr::isys::meta, Ie, u001, u001),
    MBPORT(sns::env::pwr::isrv::meta, Is, u01, u01),
    MBPORT(sns::env::pwr::ipld::meta, Ip, u01, u01),
    MBPORT(sns::env::eng::current::meta, Im, u1, u1),

    MBPORT(meta_void, ilsb_armed, bit, bit),
    MBPORT(meta_void, ilsb_approach, bit, bit),
    MBPORT(meta_void, ilsb_offset, bit, bit),
    MBPORT(meta_void, ilsb_platform, bit, bit),
    MBPORT(meta_void, ils_IR, u001, u001),
    MBPORT(meta_void, ils_RF, u001, u001),
    MBPORT(meta_void, ils_HDG, f2, f2),
    MBPORT(meta_void, ils_DME, f2, f2),
    MBPORT(meta_void, ils_heading, f2, f2),
    MBPORT(meta_void, ils_altitude, f2, f2),

    MBPORT(sns::nav::las::lat::meta, platform_lat, f4, f4),
    MBPORT(sns::nav::las::lon::meta, platform_lon, f4, f4),
    MBPORT(sns::nav::las::hmsl::meta, platform_hmsl, f4, f4),
    MBPORT(sns::nav::las::vn::meta, platform_Vnorth, f2, f2),
    MBPORT(sns::nav::las::ve::meta, platform_Veast, f2, f2),
    MBPORT(sns::nav::las::vd::meta, platform_Vdown, f2, f2),
    MBPORT(sns::nav::las::yaw::meta, platform_hdg, f2, f2),

    MBPORT(sns::nav::tcas::dist::meta, range, f2, f2),
    MBPORT(sns::nav::las::vx::meta, radar_Vx, f2, f2),
    MBPORT(sns::nav::las::vy::meta, radar_Vy, f2, f2),
    MBPORT(sns::nav::las::vz::meta, radar_Vz, f2, f2),
    MBPORT(sns::nav::las::dx::meta, radar_dx, f2, f2),
    MBPORT(sns::nav::las::dy::meta, radar_dy, f2, f2),
    MBPORT(sns::nav::las::dz::meta, radar_dz, f2, f2),

    MBPORT(cmd::nav::ctr::mode::meta, mode, u1, u1),
    MBPORT(state::nav::ctr::rc::meta, status_rc, bit, bit),
    MBPORT(state::nav::pos::status::meta, status_gps, bit, bit),
    MBPORT(state::nav::home::status::meta, status_home, bit, bit),
    MBPORT(state::nav::pos::agl_status::meta, status_agl, bit, bit),
    MBPORT(state::env::status::uplink::meta, status_modem, bit, bit),
    MBPORT(state::nav::pos::landed::meta, status_landed, bit, bit),
    MBPORT(sns::nav::agl::ground::meta, status_touch, bit, bit),

    MBPORT(state::env::status::pwr::meta, error_power, bit, bit),
    MBPORT(state::env::status::pt::meta, error_cas, bit, bit),
    MBPORT(state::env::status::ps::meta, error_pstatic, bit, bit),
    MBPORT(state::env::status::gyro::meta, error_gyro, bit, bit),
    MBPORT(state::env::status::rpm::meta, error_rpm, bit, bit),

    MBPORT(meta_void, "cmode_dlhd", bit, bit),
    MBPORT(cmd::nav::opt::thrcut::meta, "cmode_thrcut", bit, bit),
    MBPORT(cmd::nav::opt::throvr::meta, "cmode_throvr", bit, bit),
    MBPORT(state::nav::ctr::hover::meta, "cmode_hover", bit, bit),
    MBPORT(cmd::nav::opt::hyaw::meta, "cmode_hyaw", bit, bit),
    MBPORT(cmd::nav::opt::ahrs::meta, "cmode_ahrs", bit, bit),
    MBPORT(cmd::nav::opt::nomag::meta, "cmode_nomag", bit, bit),

    MBPORT(ctr::env::pwr::ap::meta, "power_ap", bit, bit),
    MBPORT(ctr::env::pwr::servo::meta, "power_servo", bit, bit),
    MBPORT(ctr::env::pwr::ignition::meta, "power_ignition", bit, bit),
    MBPORT(ctr::env::pwr::payload::meta, "power_payload", bit, bit),
    MBPORT(ctr::env::pwr::agl::meta, "power_agl", bit, bit),
    MBPORT(ctr::env::pwr::xpdr::meta, "power_xpdr", bit, bit),

    MBPORT(ctr::env::sw::starter::meta, "sw_starter", bit, bit),
    MBPORT(ctr::env::light::nav::meta, "sw_lights", bit, bit),
    MBPORT(ctr::env::light::taxi::meta, "sw_taxi", bit, bit),
    MBPORT(ctr::env::pwr::ice::meta, "sw_ice", bit, bit),
    MBPORT(ctr::env::usr::ub1::meta, "sw_sw1", bit, bit),
    MBPORT(ctr::env::usr::ub2::meta, "sw_sw2", bit, bit),
    MBPORT(ctr::env::usr::ub3::meta, "sw_sw3", bit, bit),
    MBPORT(ctr::env::usr::ub4::meta, "sw_sw4", bit, bit),

    MBPORT(sns::env::btn::shutdown::meta, sb_shutdown, bit, bit),
    MBPORT(state::env::status::ers::meta, sb_ers_err, bit, bit),
    MBPORT(sns::env::btn::noers::meta, sb_ers_disarm, bit, bit),
    MBPORT(state::env::status::eng::meta, sb_eng_err, bit, bit),
    MBPORT(state::env::status::bat::meta, sb_bat_err, bit, bit),
    MBPORT(state::env::status::gen::meta, sb_gen_err, bit, bit),

    MBPORT(state::nav::wpt::wpidx::meta, wpidx, u1, u1),
    MBPORT(state::nav::wpt::rwidx::meta, rwidx, u1, u1),
    MBPORT(state::nav::wpt::twidx::meta, twidx, u1, u1),
    MBPORT(state::nav::wpt::piidx::meta, piidx, u1, u1),
    MBPORT(meta_void, midx, u1, u1),
    MBPORT(state::nav::ctr::thdg::meta, tgHDG, f2, f2),
    MBPORT(state::nav::ctr::radius::meta, turnR, f2, f2),
    MBPORT(state::nav::ctr::delta::meta, delta, f2, f2),
    MBPORT(state::nav::ctr::loops::meta, loops, u1, u1),
    MBPORT(state::nav::wpt::eta::meta, ETA, u4, u4),
    MBPORT(state::nav::ctr::mtype::meta, mtype, u1, u1),

    MBPORT(state::nav::est::wspd::meta, windSpd, f2, u01),
    MBPORT(state::nav::est::whdg::meta, windHdg, f2, f2),
    MBPORT(state::nav::est::ktas::meta, cas2tas, f2, u001),

    MBPORT(cmd::nav::ctr::adj::meta, rwAdj, s1, s1),

    MBPORT(sns::nav::gps::sv::meta, gps_SV, u1, u1),
    MBPORT(sns::nav::gps::su::meta, gps_SU, u1, u1),
    MBPORT(sns::nav::gps::jam::meta, gps_jcw, u1, u1),
    MBPORT(meta_void, gps_jstate, u1, u1),

    MBPORT(state::nav::home::lat::meta, home_lat, f4, f4),
    MBPORT(state::nav::home::lon::meta, home_lon, f4, f4),
    MBPORT(state::nav::home::hmsl::meta, home_hmsl, f4, f4),
    MBPORT(state::nav::home::altps::meta, altps_gnd, f4, f4),

    MBPORT(meta_void, errcode, u1, u1),

    MBPORT(state::nav::est::stab::meta, stab, f2, u001),

    MBPORT(state::env::pld::cam_roll::meta, cam_roll, f4, f2),
    MBPORT(state::env::pld::cam_pitch::meta, cam_pitch, f4, f2),
    MBPORT(state::env::pld::cam_yaw::meta, cam_yaw, f4, f2),
    MBPORT(state::env::pld::turret_pitch::meta, turret_pitch, f4, f2),
    MBPORT(state::env::pld::turret_yaw::meta, turret_heading, f4, f2),
    MBPORT(meta_void, turret, u1, u1),

    MBPORT(state::env::usr::ub1::meta, userb_1, bit, bit),
    MBPORT(state::env::usr::ub2::meta, userb_2, bit, bit),
    MBPORT(state::env::usr::ub3::meta, userb_3, bit, bit),
    MBPORT(state::env::usr::ub4::meta, userb_4, bit, bit),
    MBPORT(state::env::usr::ub5::meta, userb_5, bit, bit),
    MBPORT(state::env::usr::ub6::meta, userb_6, bit, bit),
    MBPORT(state::env::usr::ub7::meta, userb_7, bit, bit),
    MBPORT(state::env::usr::ub8::meta, userb_8, bit, bit),
    MBPORT(state::env::usr::u1::meta, user1, f4, f2),
    MBPORT(state::env::usr::u2::meta, user2, f4, f2),
    MBPORT(state::env::usr::u3::meta, user3, f4, f2),
    MBPORT(state::env::usr::u4::meta, user4, f4, f2),
    MBPORT(state::env::usr::u5::meta, user5, f4, f2),
    MBPORT(state::env::usr::u6::meta, user6, f4, f2),

    MBPORT(state::env::haps::roll1::meta, ls_roll, f2, f2),
    MBPORT(state::env::haps::pitch1::meta, ls_pitch, f2, f2),
    MBPORT(state::env::haps::cpitch1::meta, ls_cp, f2, f2),
    MBPORT(state::env::haps::spd1::meta, ls_spd, f2, f2),
    MBPORT(state::env::haps::ail1::meta, ls_ail, f2, f2),
    MBPORT(state::env::haps::roll2::meta, rs_roll, f2, f2),
    MBPORT(state::env::haps::pitch2::meta, rs_pitch, f2, f2),
    MBPORT(state::env::haps::cpitch2::meta, rs_cp, f2, f2),
    MBPORT(state::env::haps::spd2::meta, rs_spd, f2, f2),
    MBPORT(state::env::haps::ail2::meta, rs_ail, f2, f2),
    MBPORT(state::env::haps::shape::meta, vshape, f2, f2),
    MBPORT(state::env::haps::cshape::meta, cmd_vshape, f2, f2),
    MBPORT(state::env::haps::roll::meta, lrs_croll, f2, f2),

    MBPORT(meta_void, local, u1, u1),

};

}; // namespace backport
}; // namespace mandala
