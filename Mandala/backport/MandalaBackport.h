#pragma once

#include <Mandala/MandalaMetaBase.h>
#include <Mandala/MandalaMetaTree.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"

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
    u10,
    u01,
    u001,
    bit,
    berr,
};

struct item_t
{
    const meta_s &meta;
    const char *alias;
    const fmt_id_t fmt_hd;
    const fmt_id_t fmt_sd;
    const uint16_t id_msb;
};

#define MBITEM(a, b, c, d) \
    { \
        a, #b, c, d, 0 \
    }

#define MBVECT(a, b, c, d, e) \
    { \
        a, #b, c, d, e \
    }

#define MBSTREAM(a, b, ...) idx_##a __VA_ARGS__

static constexpr const uint8_t uid_base = 32;

static constexpr const meta_s meta_void = cmd::env::nmt::search::meta;

typedef enum {
    MBSTREAM(service, "Service packet <node_sn>,<cmd>,<data..>", = 0),
    //special protocols
    MBSTREAM(xpdr, "Transponder data <xpdr>"),
    MBSTREAM(ident, "Identification <ident>"),
    MBSTREAM(dlink, "Datalink <squawk>,<var_idx>,<data..>"),

    MBSTREAM(ping, "Ping packet, no reply expected", = 8),
    MBSTREAM(sim, "Simulator data <var_idx>,<data..>"),
    MBSTREAM(hid, "HID data <var_idx>,<data..>"),
    MBSTREAM(jsexec, "Execute JS script <script text>"),
    //mandala
    MBSTREAM(downstream, "Downlink stream <stream>", = 16),
    MBSTREAM(uplink, "Uplink wrapped packet <var_idx>,<data..>"),
    MBSTREAM(mission, "Mission data <packed mission>"),
    MBSTREAM(set, "Set component <var_idx>,<msk>,<float/byte>"),
    MBSTREAM(data, "Port serial data <port_id>,<data..>"),
    MBSTREAM(ldata, "Port data local <port_id>,<data..>"),
    MBSTREAM(can, "Auxilary CAN data <ID32 b31=IDE>,<data..>"),
    MBSTREAM(vmexec, "Execute VM script <@function>"),
    //other & temporary
    MBSTREAM(prio, "Priority assigned var <nodeID>,<prio>,<var_idx>,<data..>"),
    MBSTREAM(active, "Autopilot active flag"),
    MBSTREAM(video, "Video stream <data>"),

    MBSTREAM(formation, "Formation flight data <packed data>"),
    MBSTREAM(vor, "VOR beacon <packed data>"),
} stream_id_t;

static constexpr const item_t items[] = {
    MBVECT(est::nav::att::roll::meta, roll, f2, f2, 0),
    MBVECT(est::nav::att::pitch::meta, pitch, f2, f2, 1),
    MBVECT(est::nav::att::yaw::meta, yaw, f2, f2, 2),

    MBVECT(est::nav::lpos::ax::meta, Ax, f2, s1, 0),
    MBVECT(est::nav::lpos::ay::meta, Ay, f2, s1, 1),
    MBVECT(est::nav::lpos::az::meta, Az, f2, s1, 2),

    MBVECT(est::nav::gyro::x::meta, p, f2, s10, 0),
    MBVECT(est::nav::gyro::y::meta, q, f2, s10, 1),
    MBVECT(est::nav::gyro::z::meta, r, f2, s10, 2),

    MBVECT(meta_void, Hx, f2, s001, 0),
    MBVECT(meta_void, Hy, f2, s001, 1),
    MBVECT(meta_void, Hz, f2, s001, 2),

    MBITEM(est::nav::air::altitude::meta, altitude, f4, f4),
    MBITEM(est::nav::air::airspeed::meta, airspeed, f2, f2),
    MBITEM(est::nav::air::vario::meta, vspeed, f2, f2),
    MBITEM(est::nav::pos::course::meta, course, f2, f2),

    MBITEM(sns::env::eng::rpm::meta, rpm, u2, u2),
    MBITEM(est::nav::pos::agl::meta, agl, f2, f2),
    MBITEM(est::nav::air::slip::meta, slip, f2, s01),
    MBITEM(est::nav::air::aoa::meta, attack, f2, s01),
    MBITEM(est::nav::tecs::vse::meta, venergy, f2, f2),
    MBITEM(est::nav::tecs::ld::meta, ldratio, f2, f2),
    MBITEM(sns::nav::air::buo::meta, buoyancy, f2, f2),

    MBITEM(ctr::nav::att::ail::meta, ctr_ailerons, f2, s001),
    MBITEM(ctr::nav::att::elv::meta, ctr_elevator, f2, s001),
    MBITEM(ctr::nav::eng::thr::meta, ctr_throttle, f2, u001),
    MBITEM(ctr::nav::att::rud::meta, ctr_rudder, f2, s001),
    MBITEM(ctr::nav::att::col::meta, ctr_collective, f2, s001),

    MBITEM(ctr::nav::str::rud::meta, ctr_steering, f2, s001),
    MBITEM(ctr::nav::wing::airbrk::meta, ctr_airbrk, f2, u001),
    MBITEM(ctr::nav::wing::flaps::meta, ctr_flaps, u001, u001),
    MBITEM(ctr::nav::str::brake::meta, ctr_brake, f2, u001),
    MBITEM(ctr::nav::eng::choke::meta, ctr_mixture, f2, u001),
    MBITEM(ctr::nav::eng::tune::meta, ctr_engine, f2, u001),
    MBITEM(ctr::nav::wing::sweep::meta, ctr_sweep, f2, s001),
    MBITEM(ctr::nav::wing::buo::meta, ctr_buoyancy, f2, s001),

    MBVECT(ctr::env::ers::launch::meta, ctrb_ers, bit, bit, 1),
    MBVECT(ctr::env::ers::rel::meta, ctrb_rel, bit, bit, 2),
    MBVECT(ctr::env::door::drop::meta, ctrb_drp, bit, bit, 4),
    MBVECT(ctr::nav::eng::fpump::meta, ctrb_pump, bit, bit, 8),
    MBVECT(sns::env::btn::starter::meta, ctrb_starter, bit, bit, 16),
    MBVECT(ctr::env::aux::horn::meta, ctrb_horn, bit, bit, 32),
    MBVECT(meta_void, ctrb_rev, bit, bit, 64),
    MBVECT(ctr::nav::str::gear::meta, ctrb_gear, bit, bit, 128),

    MBITEM(meta_void, ctr_brakeL, f2, u001),
    MBITEM(meta_void, ctr_brakeR, f2, u001),

    MBVECT(cmd::nav::att::roll::meta, cmd_roll, f2, f2, 0),
    MBVECT(cmd::nav::att::pitch::meta, cmd_pitch, f2, f2, 1),
    MBVECT(cmd::nav::att::yaw::meta, cmd_yaw, f2, f2, 2),
    MBVECT(cmd::nav::pos::n::meta, cmd_north, f2, f2, 0),
    MBVECT(cmd::nav::pos::e::meta, cmd_east, f2, f2, 1),
    MBITEM(cmd::nav::pos::course::meta, cmd_course, f2, f2),
    MBITEM(cmd::nav::aux::rpm::meta, cmd_rpm, u2, u2),
    MBITEM(cmd::nav::air::altitude::meta, cmd_altitude, f2, f2),
    MBITEM(cmd::nav::air::airspeed::meta, cmd_airspeed, f2, f2),
    MBITEM(cmd::nav::air::vario::meta, cmd_vspeed, f2, s01),
    MBITEM(cmd::nav::air::slip::meta, cmd_slip, f2, s1),

    MBVECT(est::nav::pos::lat::meta, gps_lat, f4, f4, 0),
    MBVECT(est::nav::pos::lon::meta, gps_lon, f4, f4, 1),
    MBVECT(est::nav::pos::hmsl::meta, gps_hmsl, f4, f4, 2),
    MBVECT(meta_void, gps_Vnorth, f2, f2, 0),
    MBVECT(meta_void, gps_Veast, f2, f2, 1),
    MBVECT(meta_void, gps_Vdown, f2, f2, 2),
    MBITEM(est::env::sys::time::meta, gps_time, u4, u4),

    MBITEM(est::env::sys::energy::meta, fuel, f4, f2),
    MBITEM(sns::env::fuel::rate::meta, frate, f4, f2),
    MBITEM(sns::env::com::rss::meta, RSS, u001, u001), //convert to 0..1

    MBITEM(sns::env::pwr::vsys::meta, Ve, f2, f2),
    MBITEM(sns::env::pwr::vsrv::meta, Vs, f2, f2),
    MBITEM(sns::env::pwr::vpld::meta, Vp, f2, f2),
    MBITEM(sns::env::eng::voltage::meta, Vm, f2, f2),
    MBITEM(sns::env::pwr::isys::meta, Ie, u001, u001),
    MBITEM(sns::env::pwr::isrv::meta, Is, u01, u01),
    MBITEM(sns::env::pwr::ipld::meta, Ip, u01, u01),
    MBITEM(sns::env::eng::current::meta, Im, u1, u1),

    MBITEM(sns::nav::air::temp::meta, AT, s1, s1),
    MBITEM(sns::env::aux::rt::meta, RT, s1, s1),
    MBITEM(sns::env::com::temp::meta, MT, s1, s1),
    MBITEM(sns::env::eng::temp::meta, ET, u1, u1),
    MBITEM(sns::env::eng::egt::meta, EGT, u10, u10),
    MBITEM(sns::env::eng::ot::meta, OT, u1, u1),
    MBITEM(sns::env::eng::op::meta, OP, u01, u01),

    MBVECT(meta_void, ilsb_armed, bit, bit, 1),
    MBVECT(meta_void, ilsb_approach, bit, bit, 2),
    MBVECT(meta_void, ilsb_offset, bit, bit, 4),
    MBVECT(meta_void, ilsb_platform, bit, bit, 8),
    MBVECT(meta_void, ils_IR, u001, u001, 0),
    MBVECT(meta_void, ils_RF, u001, u001, 1),
    MBITEM(meta_void, ils_HDG, f2, f2),
    MBITEM(meta_void, ils_DME, f2, f2),
    MBVECT(meta_void, ils_heading, f2, f2, 0),
    MBVECT(meta_void, ils_altitude, f2, f2, 1),

    MBVECT(sns::nav::pfm::lat::meta, platform_lat, f4, f4, 0),
    MBVECT(sns::nav::pfm::lon::meta, platform_lon, f4, f4, 1),
    MBVECT(sns::nav::pfm::hmsl::meta, platform_hmsl, f4, f4, 2),
    MBVECT(sns::nav::pfm::vn::meta, platform_Vnorth, f2, f2, 0),
    MBVECT(sns::nav::pfm::ve::meta, platform_Veast, f2, f2, 1),
    MBVECT(sns::nav::pfm::vd::meta, platform_Vdown, f2, f2, 2),
    MBITEM(sns::nav::pfm::yaw::meta, platform_hdg, f2, f2),

    MBITEM(sns::nav::tcas::dist::meta, range, f2, f2),
    MBVECT(sns::nav::las::vx::meta, radar_Vx, f2, f2, 0),
    MBVECT(sns::nav::las::vy::meta, radar_Vy, f2, f2, 1),
    MBVECT(sns::nav::las::vz::meta, radar_Vz, f2, f2, 2),
    MBVECT(sns::nav::las::dx::meta, radar_dx, f2, f2, 0),
    MBVECT(sns::nav::las::dy::meta, radar_dy, f2, f2, 1),
    MBVECT(sns::nav::las::dz::meta, radar_dz, f2, f2, 2),

    MBITEM(cmd::nav::op::stage::meta, stage, u1, u1),
    MBITEM(cmd::nav::op::mode::meta, mode, u1, u1),
    MBVECT(cmd::nav::rc::ovr::meta, status_rc, bit, bit, 1),
    MBVECT(est::env::status::gps::meta, status_gps, bit, bit, 2),
    MBVECT(est::nav::ref::status::meta, status_home, bit, bit, 4),
    MBVECT(est::env::status::agl::meta, status_agl, bit, bit, 8),
    MBVECT(est::env::status::uplink::meta, status_modem, bit, bit, 16),
    MBVECT(est::nav::air::status::meta, status_landed, bit, bit, 32),
    MBVECT(sns::nav::agl::ground::meta, status_touch, bit, bit, 64),

    MBVECT(est::env::status::pwr::meta, error_power, berr, berr, 1),
    MBVECT(est::env::status::pt::meta, error_cas, berr, berr, 2),
    MBVECT(est::env::status::ps::meta, error_pstatic, berr, berr, 4),
    MBVECT(est::env::status::imu::meta, error_gyro, berr, berr, 8),
    MBVECT(est::env::status::eng::meta, error_rpm, berr, berr, 16),

    MBVECT(meta_void, cmode_dlhd, bit, bit, 1),
    MBVECT(cmd::nav::opt::thrcut::meta, cmode_thrcut, bit, bit, 2),
    MBVECT(cmd::nav::opt::throvr::meta, cmode_throvr, bit, bit, 4),
    MBVECT(est::nav::ctr::hover::meta, cmode_hover, bit, bit, 8),
    MBVECT(cmd::nav::opt::hyaw::meta, cmode_hyaw, bit, bit, 16),
    MBVECT(cmd::nav::opt::ahrs::meta, cmode_ahrs, bit, bit, 32),
    MBVECT(cmd::nav::opt::nomag::meta, cmode_nomag, bit, bit, 64),

    MBVECT(ctr::env::pwr::ap::meta, power_ap, bit, bit, 1),
    MBVECT(ctr::env::pwr::servo::meta, power_servo, bit, bit, 2),
    MBVECT(ctr::env::pwr::ignition::meta, power_ignition, bit, bit, 4),
    MBVECT(ctr::env::pwr::payload::meta, power_payload, bit, bit, 8),
    MBVECT(ctr::env::pwr::agl::meta, power_agl, bit, bit, 16),
    MBVECT(ctr::env::pwr::xpdr::meta, power_xpdr, bit, bit, 32),

    MBVECT(ctr::nav::eng::starter::meta, sw_starter, bit, bit, 1),
    MBVECT(ctr::env::light::nav::meta, sw_lights, bit, bit, 2),
    MBVECT(ctr::env::light::taxi::meta, sw_taxi, bit, bit, 4),
    MBVECT(ctr::env::pwr::ice::meta, sw_ice, bit, bit, 8),
    MBVECT(ctr::env::usr::ub1::meta, sw_sw1, bit, bit, 16),
    MBVECT(ctr::env::usr::ub2::meta, sw_sw2, bit, bit, 32),
    MBVECT(ctr::env::usr::ub3::meta, sw_sw3, bit, bit, 64),
    MBVECT(ctr::env::usr::ub4::meta, sw_sw4, bit, bit, 128),

    MBVECT(sns::env::btn::shutdown::meta, sb_shutdown, bit, bit, 1),
    MBVECT(est::env::status::ers::meta, sb_ers_err, berr, berr, 2),
    MBVECT(sns::env::btn::noers::meta, sb_ers_disarm, bit, bit, 4),
    MBVECT(meta_void, sb_eng_err, berr, berr, 8),
    MBVECT(est::env::status::bat::meta, sb_bat_err, berr, berr, 16),
    MBVECT(est::env::status::gen::meta, sb_gen_err, berr, berr, 32),

    MBITEM(est::nav::wpt::wpidx::meta, wpidx, u1, u1),
    MBITEM(est::nav::wpt::rwidx::meta, rwidx, u1, u1),
    MBITEM(est::nav::wpt::twidx::meta, twidx, u1, u1),
    MBITEM(est::nav::wpt::piidx::meta, piidx, u1, u1),
    MBITEM(meta_void, midx, u1, u1),
    MBITEM(est::nav::ctr::thdg::meta, tgHDG, f2, f2),
    MBITEM(est::nav::ctr::radius::meta, turnR, f2, f2),
    MBITEM(est::nav::ctr::delta::meta, delta, f2, f2),
    MBITEM(est::nav::ctr::loops::meta, loops, u1, u1),
    MBITEM(est::nav::wpt::eta::meta, ETA, u4, u4),
    MBITEM(est::nav::ctr::mtype::meta, mtype, u1, u1),

    MBITEM(est::env::aux::wspd::meta, windSpd, f2, u01),
    MBITEM(est::env::aux::whdg::meta, windHdg, f2, f2),
    MBITEM(est::nav::tecs::ktas::meta, cas2tas, f2, u001),

    MBITEM(cmd::nav::op::adj::meta, rwAdj, s1, s1),

    MBITEM(sns::nav::gps::sv::meta, gps_SV, u1, u1),
    MBITEM(sns::nav::gps::su::meta, gps_SU, u1, u1),
    MBITEM(sns::nav::gps::emi::meta, gps_jcw, u1, u1),
    MBITEM(meta_void, gps_jstate, u1, u1),

    MBVECT(est::nav::ref::lat::meta, home_lat, f4, f4, 0),
    MBVECT(est::nav::ref::lon::meta, home_lon, f4, f4, 1),
    MBVECT(est::nav::ref::hmsl::meta, home_hmsl, f4, f4, 2),
    MBITEM(est::nav::ref::altps::meta, altps_gnd, f4, f4),

    MBITEM(meta_void, errcode, u1, u1),

    MBITEM(est::nav::tecs::stab::meta, stab, f2, u001),

    MBVECT(est::env::cam::roll::meta, cam_roll, f4, f2, 0),
    MBVECT(est::env::cam::pitch::meta, cam_pitch, f4, f2, 1),
    MBVECT(est::env::cam::yaw::meta, cam_yaw, f4, f2, 2),
    MBVECT(est::env::turret::pitch::meta, turret_pitch, f4, f2, 0),
    MBVECT(est::env::turret::yaw::meta, turret_heading, f4, f2, 1),
    MBVECT(meta_void, turret_armed, bit, bit, 1),
    MBVECT(meta_void, turret_shoot, bit, bit, 2),
    MBVECT(meta_void, turret_reload, bit, bit, 4),
    MBVECT(meta_void, turret_sw1, bit, bit, 8),
    MBVECT(meta_void, turret_sw2, bit, bit, 16),
    MBVECT(meta_void, turret_sw3, bit, bit, 32),
    MBVECT(meta_void, turret_sw4, bit, bit, 64),

    MBVECT(est::env::usr::ub1::meta, userb_1, bit, bit, 1),
    MBVECT(est::env::usr::ub2::meta, userb_2, bit, bit, 2),
    MBVECT(est::env::usr::ub3::meta, userb_3, bit, bit, 4),
    MBVECT(est::env::usr::ub4::meta, userb_4, bit, bit, 8),
    MBVECT(est::env::usr::ub5::meta, userb_5, bit, bit, 16),
    MBVECT(est::env::usr::ub6::meta, userb_6, bit, bit, 32),
    MBVECT(est::env::usr::ub7::meta, userb_7, bit, bit, 64),
    MBVECT(est::env::usr::ub8::meta, userb_8, bit, bit, 128),
    MBITEM(est::env::usr::u1::meta, user1, f4, f2),
    MBITEM(est::env::usr::u2::meta, user2, f4, f2),
    MBITEM(est::env::usr::u3::meta, user3, f4, f2),
    MBITEM(est::env::usr::u4::meta, user4, f4, f2),
    MBITEM(est::env::usr::u5::meta, user5, f4, f2),
    MBITEM(est::env::usr::u6::meta, user6, f4, f2),

    MBVECT(est::env::haps::roll1::meta, ls_roll, f2, f2, 0),
    MBVECT(est::env::haps::pitch1::meta, ls_pitch, f2, f2, 1),
    MBVECT(est::env::haps::cpitch1::meta, ls_cp, f2, f2, 2),
    MBITEM(est::env::haps::spd1::meta, ls_spd, f2, f2),
    MBITEM(est::env::haps::ail1::meta, ls_ail, f2, f2),
    MBVECT(est::env::haps::roll2::meta, rs_roll, f2, f2, 0),
    MBVECT(est::env::haps::pitch2::meta, rs_pitch, f2, f2, 1),
    MBVECT(est::env::haps::cpitch2::meta, rs_cp, f2, f2, 2),
    MBITEM(est::env::haps::spd2::meta, rs_spd, f2, f2),
    MBITEM(est::env::haps::ail2::meta, rs_ail, f2, f2),
    MBITEM(est::env::haps::shape::meta, vshape, f2, f2),
    MBITEM(est::env::haps::cshape::meta, cmd_vshape, f2, f2),
    MBITEM(est::env::haps::roll::meta, lrs_croll, f2, f2),

    MBITEM(meta_void, local, u1, u1),

    MBVECT(meta_void, pos_north, f4, f4, 0),
    MBVECT(meta_void, pos_east, f4, f4, 1),
    MBVECT(meta_void, vel_north, f4, f4, 0),
    MBVECT(meta_void, vel_east, f4, f4, 1),
    MBITEM(est::nav::ref::hdg::meta, homeHDG, f4, f4),
    MBITEM(est::nav::ref::dist::meta, dHome, f4, f4),
    MBITEM(est::nav::wpt::dist::meta, dWPT, f4, f4),
    MBVECT(est::nav::lpos::vx::meta, Vx, f4, f4, 0),
    MBVECT(est::nav::lpos::vy::meta, Vy, f4, f4, 1),
    MBVECT(est::nav::lpos::dx::meta, dx, f4, f4, 0),
    MBVECT(est::nav::lpos::dy::meta, dy, f4, f4, 1),
    MBITEM(est::nav::pos::speed::meta, gSpeed, f4, f4),
    MBITEM(est::nav::wpt::hdg::meta, wpHDG, f4, f4),
    MBITEM(est::nav::ctr::tdist::meta, rwDelta, f4, f4),
    MBITEM(est::nav::ctr::tvel::meta, rwDV, f4, f4),

    MBITEM(meta_void, dl_period, u2, u2),
    MBITEM(meta_void, dl_timestamp, u4, u4),

    MBITEM(sns::nav::ps::altitude::meta, altps, f4, f4),
    MBITEM(sns::nav::ps::vspeed::meta, vario, f4, f4),
    MBITEM(sns::nav::pt::acc::meta, vcas, f4, f4),
    MBITEM(meta_void, denergy, f4, f4),

    MBITEM(cmd::nav::rc::ovr::meta, rc_override, u1, u1),
    MBITEM(cmd::nav::rc::roll::meta, rc_roll, s001, s001),
    MBITEM(cmd::nav::rc::pitch::meta, rc_pitch, s001, s001),
    MBITEM(cmd::nav::rc::thr::meta, rc_throttle, u001, u001),
    MBITEM(cmd::nav::rc::yaw::meta, rc_yaw, s001, s001),

    MBITEM(cmd::nav::cam::ch::meta, cam_ch, u1, u1),
    MBITEM(cmd::nav::gimbal::mode::meta, cam_mode, u1, u1),
    MBVECT(cmd::nav::gimbal::roll::meta, camcmd_roll, f4, f4, 0),
    MBVECT(cmd::nav::gimbal::pitch::meta, camcmd_pitch, f4, f4, 1),
    MBVECT(cmd::nav::gimbal::yaw::meta, camcmd_yaw, f4, f4, 2),
    MBITEM(cmd::nav::cam::zoom::meta, cam_zoom, u001, u001),
    MBITEM(cmd::nav::cam::focus::meta, cam_focus, u001, u001),
    MBVECT(cmd::nav::gimbal::broll::meta, cambias_roll, f4, f4, 0),
    MBVECT(cmd::nav::gimbal::bpitch::meta, cambias_pitch, f4, f4, 1),
    MBVECT(cmd::nav::gimbal::byaw::meta, cambias_yaw, f4, f4, 2),

    MBVECT(cmd::nav::cam::pf::meta, cam_opt_PF, bit, bit, 1),
    MBVECT(cmd::nav::cam::nir::meta, cam_opt_NIR, bit, bit, 2),
    MBVECT(meta_void, cam_opt_DSP, bit, bit, 4),
    MBVECT(cmd::nav::cam::fm::meta, cam_opt_FMI, bit, bit, 8),
    MBVECT(cmd::nav::cam::ft::meta, cam_opt_FM, bit, bit, 16),
    MBVECT(cmd::nav::cam::range::meta, cam_opt_laser, bit, bit, 32),
    MBITEM(meta_void, cam_src, u1, u1),

    MBVECT(cmd::nav::gimbal::lat::meta, cam_lat, f4, f4, 0),
    MBVECT(cmd::nav::gimbal::lon::meta, cam_lon, f4, f4, 1),
    MBVECT(cmd::nav::gimbal::hmsl::meta, cam_hmsl, f4, f4, 2),
    MBVECT(ctr::env::cam::roll::meta, camctr_roll, f4, f4, 0),
    MBVECT(ctr::env::cam::pitch::meta, camctr_pitch, f4, f4, 1),
    MBVECT(ctr::env::cam::yaw::meta, camctr_yaw, f4, f4, 2),

    MBITEM(cmd::nav::cam::mode::meta, cams, u1, u1),
    MBVECT(ctr::env::cam::shot::meta, cam_ctrb_shtr, bit, bit, 1),
    MBVECT(ctr::env::cam::arm::meta, cam_ctrb_arm, bit, bit, 2),
    MBVECT(ctr::env::cam::rec::meta, cam_ctrb_rec, bit, bit, 4),
    MBVECT(ctr::env::cam::zin::meta, cam_ctrb_zin, bit, bit, 8),
    MBVECT(ctr::env::cam::zout::meta, cam_ctrb_zout, bit, bit, 16),
    MBVECT(ctr::env::cam::aux::meta, cam_ctrb_aux, bit, bit, 32),
    MBITEM(cmd::nav::cam::dshot::meta, cam_tperiod, u2, u2),
    MBITEM(meta_void, cam_timestamp, u4, u4),

    MBVECT(cmd::nav::turret::pitch::meta, turretcmd_pitch, f4, f4, 0),
    MBVECT(cmd::nav::turret::yaw::meta, turretcmd_yaw, f4, f4, 1),
    MBVECT(ctr::env::turret::roll::meta, turretctr_roll, f4, f4, 0),
    MBVECT(ctr::env::turret::pitch::meta, turretctr_pitch, f4, f4, 1),
    MBVECT(ctr::env::turret::yaw::meta, turretctr_yaw, f4, f4, 2),
    MBITEM(cmd::nav::turret::mode::meta, turret_mode, u1, u1),
    MBVECT(sns::env::turret::pitch::meta, turretenc_pitch, f4, f4, 0),
    MBVECT(sns::env::turret::yaw::meta, turretenc_yaw, f4, f4, 1),

    MBVECT(cmd::nav::ats::pitch::meta, atscmd_pitch, f4, f4, 0),
    MBVECT(cmd::nav::ats::yaw::meta, atscmd_yaw, f4, f4, 1),
    MBVECT(ctr::env::ats::pitch::meta, atsctr_pitch, f4, f4, 0),
    MBVECT(ctr::env::ats::yaw::meta, atsctr_yaw, f4, f4, 1),
    MBVECT(sns::env::com::pitch::meta, atsenc_pitch, f4, f4, 0),
    MBVECT(sns::env::com::yaw::meta, atsenc_yaw, f4, f4, 1),
    MBITEM(cmd::nav::ats::mode::meta, ats_mode, u1, u1),

    MBITEM(meta_void, gcu_RSS, u001, u001),
    MBITEM(meta_void, gcu_Ve, u01, u01),
    MBITEM(meta_void, gcu_MT, s1, s1),

};

}; // namespace backport
}; // namespace mandala

#pragma GCC diagnostic pop
