#pragma once

#include "TelemetryFormat.h"

#include <Mandala/MandalaMetaTree.h>
#include <constexpr_util.h>

namespace xbus {
namespace telemetry {

using namespace mandala;

#pragma pack(1)
struct ds_field_s
{
    xbus::pid_s pid;
    fmt_e fmt : 8;
};
static_assert(sizeof(ds_field_s) == 3, "size error");

constexpr fmt_e default_fmt(type_id_e type, units_e units)
{
    switch (units) {
    case units_none:
        break;

    case units_A:
        return fmt_byte_01;
    case units_Aph:
        return fmt_byte;
    case units_bar:
        return fmt_byte_01;
    case units_C:
        return fmt_sbyte;
    case units_kg:
        return fmt_f16;
    case units_deg:
        return fmt_rad;
    case units_degps:
        return fmt_sbyte_001;
    case units_degps_sq:
        return fmt_sbyte_001;
    case units_K:
        return fmt_f16;
    case units_kgpm_cb:
        return fmt_f16;
    case units_kPa:
        return fmt_f16;
    case units_l:
        return fmt_byte;
    case units_LpD:
        return fmt_f16;
    case units_lph:
        return fmt_byte_01;
    case units_m:
        return fmt_f16;
    case units_mbar:
        return fmt_f16;
    case units_mps:
        return fmt_f16;
    case units_mps_sq:
        return fmt_sbyte;
    case units_ms:
        break;
    case units_N:
        return fmt_sbyte;
    case units_Nm:
        return fmt_byte;
    case units_Pa:
        return fmt_f16;
    case units_percent:
        return fmt_byte;
    case units_rad_sq:
        return fmt_byte_01;
    case units_rpm:
        break;
    case units_s:
        break;
    case units_su:
        return fmt_sbyte_u;
    case units_u:
        return fmt_byte_u;
    case units_V:
        return fmt_f16;
    }

    switch (type) {
    default:
        return fmt_none;
    case type_real:
        return fmt_real;
    case type_dword:
        return fmt_word;
    case type_word:
        return fmt_dword;
    case type_byte:
        return fmt_byte;
    case type_option:
        return fmt_opt;
    }
}

template<class M>
constexpr ds_field_s ds(fmt_e _fmt = default_fmt(M::meta.type_id, static_cast<units_e>(M::meta.units_id)), seq_e _seq = seq_always, xbus::pri_e _pri = xbus::pri_final)
{
    return ds_field_s{xbus::pid_s(M::uid, _pri, _seq), _fmt};
}

static constexpr const ds_field_s dataset_default[] = {

    // est

    ds<est::nav::att::status>(fmt_opt),
    ds<est::nav::att::roll>(fmt_rad),
    ds<est::nav::att::pitch>(fmt_rad2),
    ds<est::nav::att::yaw>(fmt_rad),
    ds<est::nav::att::mag>(fmt_opt),
    ds<est::nav::att::rest>(fmt_bit),

    ds<est::nav::gyro::x>(fmt_f16), //fmt_sbyte_01), //727 deg/s
    ds<est::nav::gyro::y>(fmt_f16), //fmt_sbyte_01),
    ds<est::nav::gyro::z>(fmt_f16), //fmt_sbyte_01),

    ds<est::nav::gyro::ax>(fmt_f16), //fmt_sbyte_01), //727 deg/s
    ds<est::nav::gyro::ay>(fmt_f16), //fmt_sbyte_01),
    ds<est::nav::gyro::az>(fmt_f16), //fmt_sbyte_01),

    ds<est::nav::acc::x>(fmt_f16), //fmt_sbyte_01),
    ds<est::nav::acc::y>(fmt_f16), //fmt_sbyte_01),
    ds<est::nav::acc::z>(fmt_f16), //fmt_sbyte_01),

    ds<est::nav::pos::status>(fmt_opt),
    ds<est::nav::pos::lat>(fmt_real),
    ds<est::nav::pos::lon>(fmt_real),
    ds<est::nav::pos::hmsl>(fmt_real),

    ds<est::nav::pos::course>(fmt_rad),
    ds<est::nav::pos::speed>(fmt_f16, seq_skip),
    ds<est::nav::pos::altitude>(fmt_real),
    ds<est::nav::pos::vspeed>(fmt_f16),
    ds<est::nav::pos::agl>(fmt_byte_01),
    ds<est::nav::pos::hsrc>(fmt_opt),

    ds<est::nav::lpos::status>(fmt_opt),
    ds<est::nav::lpos::ax>(fmt_sbyte_01),
    ds<est::nav::lpos::ay>(fmt_sbyte_01),
    ds<est::nav::lpos::az>(fmt_f16),

    ds<est::nav::ref::status>(fmt_bit),
    ds<est::nav::ref::lat>(fmt_real, seq_scheduled),
    ds<est::nav::ref::lon>(fmt_real, seq_scheduled),
    ds<est::nav::ref::hmsl>(fmt_real, seq_scheduled),
    ds<est::nav::ref::altitude>(fmt_real, seq_scheduled),

    ds<est::nav::air::airspeed>(fmt_f16),
    ds<est::nav::air::slip>(fmt_sbyte_01),
    ds<est::nav::air::aoa>(fmt_sbyte_01),

    ds<est::nav::air::ld>(fmt_f16, seq_rare),
    ds<est::nav::air::stab>(fmt_byte_u),
    ds<est::nav::air::vse>(fmt_sbyte_01),
    ds<est::nav::air::keas>(fmt_byte_001, seq_rare),
    ds<est::nav::air::ktas>(fmt_f16, seq_rare),
    ds<est::nav::air::stall>(fmt_opt),

    ds<est::env::wind::status>(fmt_bit),
    ds<est::env::wind::speed>(fmt_byte_01, seq_rare),
    ds<est::env::wind::heading>(fmt_rad, seq_rare),

    ds<est::nav::wpt::eta>(fmt_word),
    ds<est::nav::wpt::delta>(fmt_f16),
    ds<est::nav::wpt::xtrack>(fmt_f16),
    ds<est::nav::wpt::thdg>(fmt_rad),
    ds<est::nav::wpt::loops>(fmt_byte),

    ds<est::env::sys::mode>(fmt_opt),
    ds<est::env::sys::health>(fmt_opt),
    ds<est::env::sys::time>(fmt_dword),
    ds<est::env::sys::ttl>(fmt_word),
    ds<est::env::sys::energy>(fmt_byte_u),

    // ctr
    ds<ctr::nav::att::ail>(fmt_sbyte_u),
    ds<ctr::nav::att::elv>(fmt_sbyte_u),
    ds<ctr::nav::att::rud>(fmt_sbyte_u),
    ds<ctr::nav::att::col>(fmt_sbyte_u),

    ds<ctr::nav::eng::thr>(fmt_byte_u),
    ds<ctr::nav::eng::starter>(fmt_bit),
    ds<ctr::nav::eng::ign1>(fmt_bit),
    ds<ctr::nav::eng::ign2>(fmt_bit),
    ds<ctr::nav::eng::fpump>(fmt_bit),

    ds<ctr::nav::wing::flaps>(fmt_byte_u),
    ds<ctr::nav::wing::airbrk>(fmt_byte_u),

    ds<ctr::nav::str::brake>(fmt_byte_u),
    ds<ctr::nav::str::gear>(fmt_bit),
    ds<ctr::nav::str::rud>(fmt_sbyte_u),

    ds<ctr::env::pwr::ap>(fmt_bit),
    ds<ctr::env::pwr::servo>(fmt_bit),
    ds<ctr::env::pwr::ignition>(fmt_bit),
    ds<ctr::env::pwr::payload>(fmt_bit),
    ds<ctr::env::pwr::agl>(fmt_bit),
    ds<ctr::env::pwr::xpdr>(fmt_bit),
    ds<ctr::env::pwr::satcom>(fmt_bit),
    ds<ctr::env::pwr::rfamp>(fmt_bit),
    ds<ctr::env::pwr::ice>(fmt_bit),
    ds<ctr::env::pwr::las>(fmt_bit),

    ds<ctr::env::aux::horn>(fmt_bit),

    ds<ctr::env::ers::launch>(fmt_bit),
    ds<ctr::env::ers::rel>(fmt_bit),

    ds<ctr::env::light::nav>(fmt_bit),
    ds<ctr::env::light::taxi>(fmt_bit),
    ds<ctr::env::light::beacon>(fmt_bit),
    ds<ctr::env::light::landing>(fmt_bit),
    ds<ctr::env::light::strobe>(fmt_bit),

    ds<ctr::env::door::main>(fmt_bit),
    ds<ctr::env::door::drop>(fmt_bit),

    // cmd

    ds<cmd::nav::proc::mode>(fmt_byte),
    ds<cmd::nav::proc::stage>(fmt_byte),
    ds<cmd::nav::proc::adj>(fmt_sbyte),
    ds<cmd::nav::proc::wp>(fmt_word),
    ds<cmd::nav::proc::rw>(fmt_word),
    ds<cmd::nav::proc::pi>(fmt_word),

    ds<cmd::nav::ahrs::inair>(fmt_bit),
    ds<cmd::nav::ahrs::nogps>(fmt_bit),
    ds<cmd::nav::ahrs::nomag>(fmt_bit),
    ds<cmd::nav::ahrs::hsrc>(fmt_opt),

    ds<cmd::nav::reg::att>(fmt_bit),
    ds<cmd::nav::reg::pos>(fmt_opt),
    ds<cmd::nav::reg::spd>(fmt_bit),
    ds<cmd::nav::reg::alt>(fmt_bit),
    ds<cmd::nav::reg::eng>(fmt_bit),
    ds<cmd::nav::reg::yaw>(fmt_opt),
    ds<cmd::nav::reg::str>(fmt_bit),
    ds<cmd::nav::reg::brk>(fmt_bit),
    ds<cmd::nav::reg::flaps>(fmt_bit),

    ds<cmd::nav::att::roll>(fmt_rad),
    ds<cmd::nav::att::pitch>(fmt_rad2),
    ds<cmd::nav::att::yaw>(fmt_rad),
    ds<cmd::nav::att::slip>(fmt_sbyte_001),

    ds<cmd::nav::pos::lat>(fmt_real, seq_skip),
    ds<cmd::nav::pos::lon>(fmt_real, seq_skip),
    ds<cmd::nav::pos::hmsl>(fmt_real, seq_skip),
    ds<cmd::nav::pos::course>(fmt_rad),
    ds<cmd::nav::pos::radius>(fmt_f16),

    ds<cmd::nav::pos::altitude>(fmt_real),
    ds<cmd::nav::pos::vspeed>(fmt_sbyte_01),
    ds<cmd::nav::pos::airspeed>(fmt_f16),
    ds<cmd::nav::pos::tecs>(fmt_byte_u),

    ds<cmd::nav::eng::cut>(fmt_bit),
    ds<cmd::nav::eng::ovr>(fmt_bit),
    ds<cmd::nav::eng::rpm>(fmt_word),

    ds<cmd::nav::rc::ovr>(fmt_bit),

    // sns
    ds<sns::nav::gyro::temp>(fmt_sbyte, seq_rare),
    ds<sns::nav::gyro::clip>(fmt_byte, seq_skip),
    ds<sns::nav::gyro::vib>(fmt_byte, seq_skip),
    ds<sns::nav::gyro::coning>(fmt_byte_01, seq_skip),

    ds<sns::nav::acc::temp>(fmt_sbyte, seq_rare),
    ds<sns::nav::acc::clip>(fmt_byte, seq_skip),
    ds<sns::nav::acc::vib>(fmt_byte, seq_skip),

    ds<sns::nav::mag::temp>(fmt_sbyte, seq_rare),
    ds<sns::nav::mag::vib>(fmt_byte, seq_skip),

    ds<sns::nav::gps::fix>(fmt_opt),
    ds<sns::nav::gps::emi>(fmt_opt),
    ds<sns::nav::gps::hacc>(fmt_byte_01),
    ds<sns::nav::gps::vacc>(fmt_byte_01),
    ds<sns::nav::gps::sacc>(fmt_byte_001),
    ds<sns::nav::gps::pdop>(fmt_byte_01),
    ds<sns::nav::gps::sv>(fmt_byte),
    ds<sns::nav::gps::su>(fmt_byte),

    ds<sns::nav::baro::status>(fmt_opt),
    ds<sns::nav::baro::temp>(fmt_sbyte, seq_rare),

    ds<sns::nav::pitot::status>(fmt_opt),
    ds<sns::nav::pitot::temp>(fmt_sbyte, seq_rare),

    ds<sns::nav::agl::status>(fmt_opt),

    ds<sns::env::eng::status>(fmt_opt),
    ds<sns::env::eng::rpm>(fmt_word, seq_rare),
    ds<sns::env::eng::temp>(fmt_byte, seq_rare),
    ds<sns::env::eng::ot>(fmt_byte, seq_rare),
    ds<sns::env::eng::egt>(fmt_byte_10, seq_rare),
    ds<sns::env::eng::egtd>(fmt_byte_10, seq_rare),
    ds<sns::env::eng::op>(fmt_byte_01, seq_rare),
    ds<sns::env::eng::map>(fmt_byte, seq_rare),
    ds<sns::env::eng::iap>(fmt_byte, seq_rare),
    ds<sns::env::eng::tc>(fmt_opt),
    ds<sns::env::eng::block>(fmt_bit),

    ds<sns::env::bat::status>(fmt_opt),
    ds<sns::env::gen::status>(fmt_opt),

    ds<sns::env::pwr::status>(fmt_opt),
    ds<sns::env::pwr::vsys>(fmt_f16, seq_rare),
    ds<sns::env::pwr::vsrv>(fmt_f16, seq_rare),
    ds<sns::env::pwr::vpld>(fmt_f16, seq_rare),

    ds<sns::env::com::status>(fmt_opt),
    ds<sns::env::com::rss>(fmt_byte_u),

    ds<sns::env::ers::status>(fmt_opt),
    ds<sns::env::ers::block>(fmt_bit),

    /*
    ds<est::env::status::uplink>(fmt_opt),
    */

    // test

    /*ds<est::env::sys::uptime>(fmt_dword),

    ds<sns::nav::gyro::x>(fmt_f16),
    ds<sns::nav::gyro::y>(fmt_f16),
    ds<sns::nav::gyro::z>(fmt_f16),
    ds<sns::nav::gyro::temp>(fmt_sbyte),
    ds<sns::nav::gyro::clip>(fmt_byte),
    ds<sns::nav::gyro::vib>(fmt_byte),
    ds<sns::nav::gyro::coning>(fmt_byte_01),*/

};

static constexpr const ds_field_s dataset_cam[] = {
    ds<est::env::cam::roll>(fmt_rad),
    ds<est::env::cam::pitch>(fmt_rad2),
    ds<est::env::cam::yaw>(fmt_rad),
    ds<est::env::cam::lat>(fmt_real),
    ds<est::env::cam::lon>(fmt_real),
    ds<est::env::cam::hmsl>(fmt_f16),

    ds<cmd::nav::cam::zoom>(fmt_byte_u),
    ds<cmd::nav::cam::focus>(fmt_byte_u),
    ds<cmd::nav::cam::range>(fmt_bit),
    ds<cmd::nav::cam::mode>(fmt_opt),

    ds<cmd::nav::gimbal::mode>(fmt_opt),
    ds<cmd::nav::gimbal::roll>(fmt_rad),
    ds<cmd::nav::gimbal::pitch>(fmt_rad2),
    ds<cmd::nav::gimbal::yaw>(fmt_rad),

    ds<sns::env::cam::range>(fmt_word, seq_rare),

};

static constexpr const ds_field_s dataset_ats[] = {
    ds<est::env::ats::roll>(fmt_rad),
    ds<est::env::ats::pitch>(fmt_rad2),
    ds<est::env::ats::yaw>(fmt_rad),

    ds<cmd::nav::ats::mode>(fmt_opt),
    ds<cmd::nav::ats::roll>(fmt_rad),
    ds<cmd::nav::ats::pitch>(fmt_rad2),
    ds<cmd::nav::ats::yaw>(fmt_rad),
};

static constexpr const ds_field_s dataset_turret[] = {
    ds<est::env::turret::roll>(fmt_rad),
    ds<est::env::turret::pitch>(fmt_rad2),
    ds<est::env::turret::yaw>(fmt_rad),
    ds<est::env::turret::lat>(fmt_real),
    ds<est::env::turret::lon>(fmt_real),
    ds<est::env::turret::hmsl>(fmt_real),

    ds<ctr::env::turret::op>(fmt_opt),

    ds<cmd::nav::turret::mode>(fmt_opt),
    ds<cmd::nav::turret::roll>(fmt_rad),
    ds<cmd::nav::turret::pitch>(fmt_rad2),
    ds<cmd::nav::turret::yaw>(fmt_rad),

    ds<sns::env::turret::status>(fmt_opt),
    ds<sns::env::turret::capacity>(fmt_byte),
};

static constexpr const ds_field_s dataset_usr[] = {
    ds<est::env::usr::u1>(fmt_f16),
    ds<est::env::usr::u2>(fmt_f16),
    ds<est::env::usr::u3>(fmt_f16),
    ds<est::env::usr::u4>(fmt_f16),
    ds<est::env::usr::u5>(fmt_f16),
    ds<est::env::usr::u6>(fmt_f16),
    ds<est::env::usr::u7>(fmt_f16),
    ds<est::env::usr::u8>(fmt_f16),
    ds<est::env::usr::ub1>(fmt_byte),
    ds<est::env::usr::ub2>(fmt_byte),
    ds<est::env::usr::ub3>(fmt_byte),
    ds<est::env::usr::ub4>(fmt_byte),
    ds<est::env::usr::ub5>(fmt_byte),
    ds<est::env::usr::ub6>(fmt_byte),
    ds<est::env::usr::ub7>(fmt_byte),
};

static constexpr const ds_field_s dataset_haps[] = {
    ds<est::env::haps::shape>(fmt_rad),
    ds<est::env::haps::cshape>(fmt_rad),
    ds<est::env::haps::roll>(fmt_rad),
    ds<est::env::haps::roll1>(fmt_rad),
    ds<est::env::haps::roll2>(fmt_rad),
    ds<est::env::haps::pitch1>(fmt_rad),
    ds<est::env::haps::pitch2>(fmt_rad),
    ds<est::env::haps::cpitch1>(fmt_rad),
    ds<est::env::haps::cpitch2>(fmt_rad),
    ds<est::env::haps::spd1>(fmt_byte_01),
    ds<est::env::haps::spd2>(fmt_byte_01),
    ds<est::env::haps::ail1>(fmt_sbyte_u),
    ds<est::env::haps::ail2>(fmt_sbyte_u),
};

static constexpr const ds_field_s dataset_redundancy[] = {
    ds<sns::nav::gyro::src>(fmt_opt),
    ds<sns::nav::gyro::cnt>(fmt_opt),
    ds<sns::nav::acc::src>(fmt_opt),
    ds<sns::nav::acc::cnt>(fmt_opt),
    ds<sns::nav::mag::src>(fmt_opt),
    ds<sns::nav::mag::cnt>(fmt_opt),
    ds<sns::nav::gps::src>(fmt_opt),
    ds<sns::nav::gps::cnt>(fmt_opt),
    ds<sns::nav::baro::src>(fmt_opt),
    ds<sns::nav::baro::cnt>(fmt_opt),
    ds<sns::nav::pitot::src>(fmt_opt),
    ds<sns::nav::pitot::cnt>(fmt_opt),
    ds<sns::nav::pitot::valid>(fmt_opt),
};

#pragma pack()

} // namespace telemetry
} // namespace xbus
