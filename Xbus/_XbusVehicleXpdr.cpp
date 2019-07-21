#include "XbusVehicleXpdr.h"

#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

XbusVehicleXpdr::XbusVehicleXpdr(uint8_t *packet_ptr)
    : XbusVehicle(packet_ptr)
{

}

bool XbusVehicleXpdr::isValid(const uint16_t packet_size) const
{
    return packet_size==(offset_payload
                         + sizeof(lat_t)
                         + sizeof(lon_t)
                         + sizeof(alt_t)
                         + sizeof(speed_t)
                         + sizeof(course_t)
                         + sizeof(mode_t));
}


void XbusVehicleXpdr::read(Xpdr &d)
{
    XbusStreamReader s(payload());
    s >> d.lat;
    s >> d.lon;
    d.alt=s.read<alt_t,float>();
    d.speed=s.read<speed_t,float>()/100.0f;
    d.course=s.read<course_t,float>()/(32768.0f/180.0f);
    s >> d.mode;
}
void XbusVehicleXpdr::write(const Xpdr &d)
{
    XbusStreamWriter s(payload());
    s << d.lat;
    s << d.lon;
    s.write<alt_t>(d.alt);
    s.write<speed_t>(d.speed*100.0f);
    s.write<course_t>(d.course*(32768.0f/180.0f));
    s << d.mode;
}
