#pragma once

#include <Mandala/MandalaMetaBase.h>
#include <Mandala/MandalaMetaTree.h>

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
    bit
};

struct item_t
{
    const meta_t &meta;
    const char *alias;
    const fmt_id_t fmt_hd;
    const fmt_id_t fmt_sd;
};

class MandalaBackport
{
public:
    explicit MandalaBackport();

    //bool decode();

private:
    static constexpr const uint8_t &uid_base = 32;
};

#define MBPORT(a, b, c, d) \
    { \
        a, #b, c, d \
    }

static constexpr const meta_t &meta_void = cmd::env::dict::search::meta;

static constexpr const item_t items[] = {
    MBPORT(est::nav::att::roll::meta, roll, f2, f2),
    MBPORT(est::nav::att::pitch::meta, pitch, f2, f2),
    MBPORT(est::nav::att::yaw::meta, yaw, f2, f2),

    MBPORT(est::nav::rel::ax::meta, Ax, f2, s1),
    MBPORT(est::nav::rel::ay::meta, Ay, f2, s1),
    MBPORT(est::nav::rel::az::meta, Az, f2, s1),

    MBPORT(est::nav::att::p::meta, p, f2, s10),
    MBPORT(est::nav::att::q::meta, q, f2, s10),
    MBPORT(est::nav::att::r::meta, r, f2, s10),

    MBPORT(sns::nav::mag::x::meta, Hx, f2, s001),
    MBPORT(sns::nav::mag::y::meta, Hy, f2, s001),
    MBPORT(sns::nav::mag::z::meta, Hz, f2, s001),

    MBPORT(est::nav::air::altitude::meta, altitude, f4, f4),
    MBPORT(est::nav::air::airspeed::meta, airspeed, f2, f2),
    MBPORT(est::nav::air::vspeed::meta, vspeed, f2, f2),
    MBPORT(est::nav::calc::course::meta, course, f2, f2),

    MBPORT(est::nav::ctr::rpm::meta, rpm, u2, u2),
    MBPORT(est::nav::air::agl::meta, agl, f2, f2),
    MBPORT(est::nav::air::slip::meta, slip, f2, s01),
    MBPORT(est::nav::air::aoa::meta, attack, f2, s01),
    MBPORT(est::nav::calc::venergy::meta, venergy, f2, f2),
    MBPORT(est::nav::calc::ld::meta, ldratio, f2, f2),
    MBPORT(sns::nav::air::buo::meta, buoyancy, f2, f2),

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

    MBPORT(est::nav::pos::lat::meta, gps_lat, f4, f4),
    MBPORT(est::nav::pos::lon::meta, gps_lon, f4, f4),
    MBPORT(est::nav::pos::hmsl::meta, gps_hmsl, f4, f4),
    MBPORT(est::nav::pos::vn::meta, gps_Vnorth, f2, f2),
    MBPORT(est::nav::pos::ve::meta, gps_Veast, f2, f2),
    MBPORT(est::nav::pos::vd::meta, gps_Vdown, f2, f2),
    MBPORT(est::env::sys::time::meta, gps_time, u4, u4),

    MBPORT(est::env::sys::energy::meta, fuel, f4, f2),
    MBPORT(sns::env::fuel::rate::meta, frate, f4, f2),
    MBPORT(sns::env::com::rss::meta, RSS, u001, u001), //convert to 0..1

    MBPORT(sns::env::pwr::vsys::meta, Ve, f2, f2),
    MBPORT(sns::env::pwr::vsrv::meta, Vs, f2, f2),
    MBPORT(sns::env::pwr::vpld::meta, Vp, f2, f2),
    MBPORT(sns::env::eng::voltage::meta, Vm, f2, f2),
    MBPORT(sns::env::pwr::isys::meta, Ie, u001, u001),
    MBPORT(sns::env::pwr::isrv::meta, Is, u01, u01),
    MBPORT(sns::env::pwr::ipld::meta, Ip, u01, u01),
    MBPORT(sns::env::eng::current::meta, Im, u1, u1),

    MBPORT(sns::nav::air::temp::meta, AT, s1, s1),
    MBPORT(sns::env::status::rt::meta, RT, s1, s1),
    MBPORT(sns::env::com::temp::meta, MT, s1, s1),
    MBPORT(sns::env::eng::temp::meta, ET, u1, u1),
    MBPORT(sns::env::eng::egt::meta, EGT, u10, u10),
    MBPORT(sns::env::eng::ot::meta, OT, u1, u1),
    MBPORT(sns::env::eng::op::meta, OP, u01, u01),

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

    MBPORT(sns::nav::pfm::lat::meta, platform_lat, f4, f4),
    MBPORT(sns::nav::pfm::lon::meta, platform_lon, f4, f4),
    MBPORT(sns::nav::pfm::hmsl::meta, platform_hmsl, f4, f4),
    MBPORT(sns::nav::pfm::vn::meta, platform_Vnorth, f2, f2),
    MBPORT(sns::nav::pfm::ve::meta, platform_Veast, f2, f2),
    MBPORT(sns::nav::pfm::vd::meta, platform_Vdown, f2, f2),
    MBPORT(sns::nav::pfm::yaw::meta, platform_hdg, f2, f2),

    MBPORT(sns::nav::tcas::dist::meta, range, f2, f2),
    MBPORT(sns::nav::las::vx::meta, radar_Vx, f2, f2),
    MBPORT(sns::nav::las::vy::meta, radar_Vy, f2, f2),
    MBPORT(sns::nav::las::vz::meta, radar_Vz, f2, f2),
    MBPORT(sns::nav::las::dx::meta, radar_dx, f2, f2),
    MBPORT(sns::nav::las::dy::meta, radar_dy, f2, f2),
    MBPORT(sns::nav::las::dz::meta, radar_dz, f2, f2),

    MBPORT(cmd::nav::ctr::stage::meta, stage, u1, u1),
    MBPORT(cmd::nav::ctr::mode::meta, mode, u1, u1),
    MBPORT(cmd::nav::rc::ovr::meta, status_rc, bit, bit),
    MBPORT(est::nav::status::pos::meta, status_gps, bit, bit),
    MBPORT(est::nav::status::ref::meta, status_home, bit, bit),
    MBPORT(est::nav::status::agl::meta, status_agl, bit, bit),
    MBPORT(est::env::status::uplink::meta, status_modem, bit, bit),
    MBPORT(est::nav::status::ap::meta, status_landed, bit, bit),
    MBPORT(sns::nav::agl::ground::meta, status_touch, bit, bit),

    MBPORT(est::env::status::pwr::meta, error_power, bit, bit),
    MBPORT(est::env::status::pt::meta, error_cas, bit, bit),
    MBPORT(est::env::status::ps::meta, error_pstatic, bit, bit),
    MBPORT(est::env::status::gyro::meta, error_gyro, bit, bit),
    MBPORT(est::env::status::rpm::meta, error_rpm, bit, bit),

    MBPORT(meta_void, cmode_dlhd, bit, bit),
    MBPORT(cmd::nav::opt::thrcut::meta, cmode_thrcut, bit, bit),
    MBPORT(cmd::nav::opt::throvr::meta, cmode_throvr, bit, bit),
    MBPORT(est::nav::ctr::hover::meta, cmode_hover, bit, bit),
    MBPORT(cmd::nav::opt::hyaw::meta, cmode_hyaw, bit, bit),
    MBPORT(cmd::nav::opt::ahrs::meta, cmode_ahrs, bit, bit),
    MBPORT(cmd::nav::opt::nomag::meta, cmode_nomag, bit, bit),

    MBPORT(ctr::env::pwr::ap::meta, power_ap, bit, bit),
    MBPORT(ctr::env::pwr::servo::meta, power_servo, bit, bit),
    MBPORT(ctr::env::pwr::ignition::meta, power_ignition, bit, bit),
    MBPORT(ctr::env::pwr::payload::meta, power_payload, bit, bit),
    MBPORT(ctr::env::pwr::agl::meta, power_agl, bit, bit),
    MBPORT(ctr::env::pwr::xpdr::meta, power_xpdr, bit, bit),

    MBPORT(ctr::env::sw::starter::meta, sw_starter, bit, bit),
    MBPORT(ctr::env::light::nav::meta, sw_lights, bit, bit),
    MBPORT(ctr::env::light::taxi::meta, sw_taxi, bit, bit),
    MBPORT(ctr::env::pwr::ice::meta, sw_ice, bit, bit),
    MBPORT(ctr::env::usr::ub1::meta, sw_sw1, bit, bit),
    MBPORT(ctr::env::usr::ub2::meta, sw_sw2, bit, bit),
    MBPORT(ctr::env::usr::ub3::meta, sw_sw3, bit, bit),
    MBPORT(ctr::env::usr::ub4::meta, sw_sw4, bit, bit),

    MBPORT(sns::env::btn::shutdown::meta, sb_shutdown, bit, bit),
    MBPORT(est::env::status::ers::meta, sb_ers_err, bit, bit),
    MBPORT(sns::env::btn::noers::meta, sb_ers_disarm, bit, bit),
    MBPORT(est::env::status::eng::meta, sb_eng_err, bit, bit),
    MBPORT(est::env::status::bat::meta, sb_bat_err, bit, bit),
    MBPORT(est::env::status::gen::meta, sb_gen_err, bit, bit),

    MBPORT(est::nav::wpt::wpidx::meta, wpidx, u1, u1),
    MBPORT(est::nav::wpt::rwidx::meta, rwidx, u1, u1),
    MBPORT(est::nav::wpt::twidx::meta, twidx, u1, u1),
    MBPORT(est::nav::wpt::piidx::meta, piidx, u1, u1),
    MBPORT(meta_void, midx, u1, u1),
    MBPORT(est::nav::ctr::thdg::meta, tgHDG, f2, f2),
    MBPORT(est::nav::ctr::radius::meta, turnR, f2, f2),
    MBPORT(est::nav::ctr::delta::meta, delta, f2, f2),
    MBPORT(est::nav::ctr::loops::meta, loops, u1, u1),
    MBPORT(est::nav::wpt::eta::meta, ETA, u4, u4),
    MBPORT(est::nav::ctr::mtype::meta, mtype, u1, u1),

    MBPORT(est::nav::calc::wspd::meta, windSpd, f2, u01),
    MBPORT(est::nav::calc::whdg::meta, windHdg, f2, f2),
    MBPORT(est::nav::calc::ktas::meta, cas2tas, f2, u001),

    MBPORT(cmd::nav::ctr::adj::meta, rwAdj, s1, s1),

    MBPORT(sns::nav::gps::sv::meta, gps_SV, u1, u1),
    MBPORT(sns::nav::gps::su::meta, gps_SU, u1, u1),
    MBPORT(sns::nav::gps::jam::meta, gps_jcw, u1, u1),
    MBPORT(meta_void, gps_jstate, u1, u1),

    MBPORT(est::nav::ref::lat::meta, home_lat, f4, f4),
    MBPORT(est::nav::ref::lon::meta, home_lon, f4, f4),
    MBPORT(est::nav::ref::hmsl::meta, home_hmsl, f4, f4),
    MBPORT(est::nav::ref::altps::meta, altps_gnd, f4, f4),

    MBPORT(meta_void, errcode, u1, u1),

    MBPORT(est::nav::calc::stab::meta, stab, f2, u001),

    MBPORT(est::env::pld::cam_roll::meta, cam_roll, f4, f2),
    MBPORT(est::env::pld::cam_pitch::meta, cam_pitch, f4, f2),
    MBPORT(est::env::pld::cam_yaw::meta, cam_yaw, f4, f2),
    MBPORT(est::env::pld::turret_pitch::meta, turret_pitch, f4, f2),
    MBPORT(est::env::pld::turret_yaw::meta, turret_heading, f4, f2),
    MBPORT(meta_void, turret_armed, bit, bit),
    MBPORT(meta_void, turret_shoot, bit, bit),
    MBPORT(meta_void, turret_reload, bit, bit),
    MBPORT(meta_void, turret_sw1, bit, bit),
    MBPORT(meta_void, turret_sw2, bit, bit),
    MBPORT(meta_void, turret_sw3, bit, bit),
    MBPORT(meta_void, turret_sw4, bit, bit),

    MBPORT(est::env::usr::ub1::meta, userb_1, bit, bit),
    MBPORT(est::env::usr::ub2::meta, userb_2, bit, bit),
    MBPORT(est::env::usr::ub3::meta, userb_3, bit, bit),
    MBPORT(est::env::usr::ub4::meta, userb_4, bit, bit),
    MBPORT(est::env::usr::ub5::meta, userb_5, bit, bit),
    MBPORT(est::env::usr::ub6::meta, userb_6, bit, bit),
    MBPORT(est::env::usr::ub7::meta, userb_7, bit, bit),
    MBPORT(est::env::usr::ub8::meta, userb_8, bit, bit),
    MBPORT(est::env::usr::u1::meta, user1, f4, f2),
    MBPORT(est::env::usr::u2::meta, user2, f4, f2),
    MBPORT(est::env::usr::u3::meta, user3, f4, f2),
    MBPORT(est::env::usr::u4::meta, user4, f4, f2),
    MBPORT(est::env::usr::u5::meta, user5, f4, f2),
    MBPORT(est::env::usr::u6::meta, user6, f4, f2),

    MBPORT(est::env::haps::roll1::meta, ls_roll, f2, f2),
    MBPORT(est::env::haps::pitch1::meta, ls_pitch, f2, f2),
    MBPORT(est::env::haps::cpitch1::meta, ls_cp, f2, f2),
    MBPORT(est::env::haps::spd1::meta, ls_spd, f2, f2),
    MBPORT(est::env::haps::ail1::meta, ls_ail, f2, f2),
    MBPORT(est::env::haps::roll2::meta, rs_roll, f2, f2),
    MBPORT(est::env::haps::pitch2::meta, rs_pitch, f2, f2),
    MBPORT(est::env::haps::cpitch2::meta, rs_cp, f2, f2),
    MBPORT(est::env::haps::spd2::meta, rs_spd, f2, f2),
    MBPORT(est::env::haps::ail2::meta, rs_ail, f2, f2),
    MBPORT(est::env::haps::shape::meta, vshape, f2, f2),
    MBPORT(est::env::haps::cshape::meta, cmd_vshape, f2, f2),
    MBPORT(est::env::haps::roll::meta, lrs_croll, f2, f2),

    MBPORT(meta_void, local, u1, u1),

    MBPORT(est::nav::pos::n::meta, pos_north, f4, f4),
    MBPORT(est::nav::pos::e::meta, pos_east, f4, f4),
    MBPORT(est::nav::pos::vn::meta, vel_north, f4, f4),
    MBPORT(est::nav::pos::ve::meta, vel_east, f4, f4),
    MBPORT(est::nav::ref::hdg::meta, homeHDG, f4, f4),
    MBPORT(est::nav::ref::dist::meta, dHome, f4, f4),
    MBPORT(est::nav::wpt::dist::meta, dWPT, f4, f4),
    MBPORT(est::nav::rel::vx::meta, Vx, f4, f4),
    MBPORT(est::nav::rel::vy::meta, Vy, f4, f4),
    MBPORT(est::nav::rel::dx::meta, dx, f4, f4),
    MBPORT(est::nav::rel::dy::meta, dy, f4, f4),
    MBPORT(est::nav::calc::speed::meta, gSpeed, f4, f4),
    MBPORT(est::nav::wpt::hdg::meta, wpHDG, f4, f4),
    MBPORT(est::nav::ctr::tdist::meta, rwDelta, f4, f4),
    MBPORT(est::nav::ctr::tvel::meta, rwDV, f4, f4),

    MBPORT(meta_void, dl_period, u2, u2),
    MBPORT(meta_void, dl_timestamp, u4, u4),

    MBPORT(sns::nav::ps::altitude::meta, altps, f4, f4),
    MBPORT(sns::nav::ps::altitude_d::meta, vario, f4, f4),
    MBPORT(sns::nav::pt::airspeed_d::meta, vcas, f4, f4),
    MBPORT(meta_void, denergy, f4, f4),

    MBPORT(cmd::nav::rc::ovr::meta, rc_override, u1, u1),
    MBPORT(cmd::nav::rc::roll::meta, rc_roll, s001, s001),
    MBPORT(cmd::nav::rc::pitch::meta, rc_pitch, s001, s001),
    MBPORT(cmd::nav::rc::throttle::meta, rc_throttle, u001, u001),
    MBPORT(cmd::nav::rc::yaw::meta, rc_yaw, s001, s001),

    MBPORT(cmd::nav::cam::ch::meta, cam_ch, u1, u1),
    MBPORT(cmd::nav::gimbal::mode::meta, cam_mode, u1, u1),
    MBPORT(cmd::nav::gimbal::roll::meta, camcmd_roll, f4, f4),
    MBPORT(cmd::nav::gimbal::pitch::meta, camcmd_pitch, f4, f4),
    MBPORT(cmd::nav::gimbal::yaw::meta, camcmd_yaw, f4, f4),
    MBPORT(cmd::nav::cam::zoom::meta, cam_zoom, u001, u001),
    MBPORT(cmd::nav::cam::focus::meta, cam_focus, u001, u001),
    MBPORT(cmd::nav::gimbal::broll::meta, cambias_roll, f4, f4),
    MBPORT(cmd::nav::gimbal::bpitch::meta, cambias_pitch, f4, f4),
    MBPORT(cmd::nav::gimbal::byaw::meta, cambias_yaw, f4, f4),

    MBPORT(cmd::nav::cam::pf::meta, cam_opt_PF, bit, bit),
    MBPORT(cmd::nav::cam::nir::meta, cam_opt_NIR, bit, bit),
    MBPORT(meta_void, cam_opt_DSP, bit, bit),
    MBPORT(cmd::nav::cam::fm::meta, cam_opt_FMI, bit, bit),
    MBPORT(cmd::nav::cam::ft::meta, cam_opt_FM, bit, bit),
    MBPORT(cmd::nav::cam::range::meta, cam_opt_laser, bit, bit),
    MBPORT(meta_void, cam_src, u1, u1),

    MBPORT(cmd::nav::gimbal::lat::meta, cam_lat, f4, f4),
    MBPORT(cmd::nav::gimbal::lon::meta, cam_lon, f4, f4),
    MBPORT(cmd::nav::gimbal::hmsl::meta, cam_hmsl, f4, f4),
    MBPORT(ctr::env::cam::roll::meta, camctr_roll, f4, f4),
    MBPORT(ctr::env::cam::pitch::meta, camctr_pitch, f4, f4),
    MBPORT(ctr::env::cam::yaw::meta, camctr_yaw, f4, f4),

    MBPORT(cmd::nav::cam::shot::meta, cams, u1, u1),
    MBPORT(ctr::env::cam::shot::meta, cam_ctrb_shtr, bit, bit),
    MBPORT(ctr::env::cam::arm::meta, cam_ctrb_arm, bit, bit),
    MBPORT(ctr::env::cam::rec::meta, cam_ctrb_rec, bit, bit),
    MBPORT(ctr::env::cam::zin::meta, cam_ctrb_zin, bit, bit),
    MBPORT(ctr::env::cam::zout::meta, cam_ctrb_zout, bit, bit),
    MBPORT(ctr::env::cam::aux::meta, cam_ctrb_aux, bit, bit),
    MBPORT(cmd::nav::cam::dshot::meta, cam_tperiod, u2, u2),
    MBPORT(meta_void, cam_timestamp, u4, u4),

    MBPORT(cmd::nav::turret::pitch::meta, turretcmd_pitch, f4, f4),
    MBPORT(cmd::nav::turret::yaw::meta, turretcmd_yaw, f4, f4),
    MBPORT(ctr::env::turret::roll::meta, turretctr_roll, f4, f4),
    MBPORT(ctr::env::turret::pitch::meta, turretctr_pitch, f4, f4),
    MBPORT(ctr::env::turret::yaw::meta, turretctr_yaw, f4, f4),
    MBPORT(cmd::nav::turret::mode::meta, turret_mode, u1, u1),
    MBPORT(sns::env::turret::pitch::meta, turretenc_pitch, f4, f4),
    MBPORT(sns::env::turret::yaw::meta, turretenc_yaw, f4, f4),

    MBPORT(cmd::nav::ats::pitch::meta, atscmd_pitch, f4, f4),
    MBPORT(cmd::nav::ats::yaw::meta, atscmd_yaw, f4, f4),
    MBPORT(ctr::env::ats::pitch::meta, atsctr_pitch, f4, f4),
    MBPORT(ctr::env::ats::yaw::meta, atsctr_yaw, f4, f4),
    MBPORT(sns::env::com::pitch::meta, atsenc_pitch, f4, f4),
    MBPORT(sns::env::com::yaw::meta, atsenc_yaw, f4, f4),
    MBPORT(cmd::nav::ats::mode::meta, ats_mode, u1, u1),

    MBPORT(meta_void, gcu_RSS, u001, u001),
    MBPORT(meta_void, gcu_Ve, u01, u01),
    MBPORT(meta_void, gcu_MT, s1, s1),

};

}; // namespace backport
}; // namespace mandala
