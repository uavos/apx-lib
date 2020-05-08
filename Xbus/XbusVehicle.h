#pragma once
#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

namespace xbus {
namespace vehicle {

typedef uint16_t squawk_t;

// Vehicle IDENT
typedef uint8_t uid_t[12]; //global unique vehicle id

typedef struct
{
    uid_t uid;

    union {
        uint32_t _raw;
        struct
        {
            bool gcs : 1; // set if Ground Control
        };
    } flags;

    static inline uint16_t psize()
    {
        return sizeof(uid_t) + sizeof(flags._raw);
    }
    inline void read(XbusStreamReader *s)
    {
        s->read(uid, sizeof(uid));
        *s >> flags._raw;
    }
    inline void write(XbusStreamWriter *s) const
    {
        s->write(uid, sizeof(uid));
        *s << flags._raw;
    }
} ident_s;

// Vehicle transponder data
typedef float lat_t;
typedef float lon_t;
typedef int16_t alt_t;
typedef uint16_t speed_t;
typedef int16_t course_t;
typedef uint8_t mode_t;

typedef struct
{
    lat_t lat;
    lon_t lon;
    float alt;
    float speed;
    float course;
    mode_t mode;

    static inline uint16_t psize()
    {
        return sizeof(lat_t)
               + sizeof(lon_t)
               + sizeof(alt_t)
               + sizeof(speed_t)
               + sizeof(course_t)
               + sizeof(mode_t);
    }

    inline void read(XbusStreamReader *s)
    {
        *s >> lat;
        *s >> lon;
        alt = s->read<alt_t, float>();
        speed = s->read<speed_t, float>() / 100.0f;
        course = s->read<course_t, float>() / (32768.0f / 180.0f);
        *s >> mode;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << lat;
        *s << lon;
        s->write<alt_t, float>(alt);
        s->write<speed_t, float>(speed * 100.0f);
        s->write<course_t, float>(course * (32768.0f / 180.0f));
        *s << mode;
    }
} xpdr_s;

} // namespace vehicle
} // namespace xbus
