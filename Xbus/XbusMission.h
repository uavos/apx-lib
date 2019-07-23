#pragma once
#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

namespace xbus {
namespace mission {


    struct Header {
        uint8_t type;     //wp,rw,scr, ..
        uint8_t option;   //left,right,line,hdg, ..

        typedef enum{ //4bits
            mi_stop=0,
            mi_wp,
            mi_rw,
            mi_tw,
            mi_pi,
            mi_action,
            mi_restricted,
            mi_emergency
        } itemtypes_t;

        static inline uint16_t psize()
        {
            return sizeof(uint8_t);
        }
        inline void read(XbusStreamReader *s)
        {
            uint8_t v;
            *s >> v;
            type=v&0x0F;
            option=v>>4;
        }
        inline void write(XbusStreamWriter *s) const
        {
            uint8_t v=(type&0x0F)|((option<<4)&0xF0);
            *s << v;
        }
    };

    struct Waypoint {
        float           lat;
        float           lon;
        int16_t         alt;

        typedef enum {
            mo_hdg,
            mo_line,
        } options_t;

        static inline uint16_t psize()
        {
            return sizeof(float)*2
                    + sizeof(int16_t);
        }
        inline void read(XbusStreamReader *s)
        {
            *s >> lat;
            *s >> lon;
            *s >> alt;
        }
        inline void write(XbusStreamWriter *s) const
        {
            *s << lat;
            *s << lon;
            *s << alt;
        }
    };


    struct Runway {
        float           lat;
        float           lon;
        int16_t         hmsl;
        int16_t         dN;
        int16_t         dE;
        uint16_t        approach;

        typedef enum {
            mo_left,
            mo_right,
        } options_t;

        static inline uint16_t psize()
        {
            return sizeof(float)*2
                    + sizeof(int16_t)
                    + sizeof(int16_t)
                    + sizeof(int16_t)
                    + sizeof(uint16_t);
        }
        inline void read(XbusStreamReader *s)
        {
            *s >> lat;
            *s >> lon;
            *s >> hmsl;
            *s >> dN;
            *s >> dE;
            *s >> approach;
        }
        inline void write(XbusStreamWriter *s) const
        {
            *s << lat;
            *s << lon;
            *s << hmsl;
            *s << dN;
            *s << dE;
            *s << approach;
        }
    };


    struct Taxiway {
        float           lat;
        float           lon;

        static inline uint16_t psize()
        {
            return sizeof(float)*2;
        }
        inline void read(XbusStreamReader *s)
        {
            *s >> lat;
            *s >> lon;
        }
        inline void write(XbusStreamWriter *s) const
        {
            *s << lat;
            *s << lon;
        }
    };

    struct Poi {
        float           lat;
        float           lon;
        int16_t         hmsl;
        int16_t         turnR;
        uint8_t         loops;
        uint16_t        timeS;

        static inline uint16_t psize()
        {
            return sizeof(float)*2
                    + sizeof(int16_t)
                    + sizeof(int16_t)
                    + sizeof(uint8_t)
                    + sizeof(uint16_t);
        }
        inline void read(XbusStreamReader *s)
        {
            *s >> lat;
            *s >> lon;
            *s >> hmsl;
            *s >> turnR;
            *s >> loops;
            *s >> timeS;
        }
        inline void write(XbusStreamWriter *s) const
        {
            *s << lat;
            *s << lon;
            *s << hmsl;
            *s << turnR;
            *s << loops;
            *s << timeS;
        }
    };


    struct Area {
        uint8_t   pointsCnt;

        struct Point {
            float   lat;
            float   lon;

            static inline uint16_t psize()
            {
                return sizeof(float)*2;
            }
            inline void read(XbusStreamReader *s)
            {
                *s >> lat;
                *s >> lon;
            }
            inline void write(XbusStreamWriter *s) const
            {
                *s << lat;
                *s << lon;
            }
        };

        static inline uint16_t psize(uint8_t pointsCnt)
        {
            return sizeof(uint8_t)+Point::psize()*pointsCnt;
        }
        inline void read(XbusStreamReader *s)
        {
            *s >> pointsCnt;
        }
        inline void write(XbusStreamWriter *s) const
        {
            *s << pointsCnt;
        }
    };




    // Actions

    struct Action {

        typedef enum {
            mo_speed,
            mo_poi,
            mo_scr,
            mo_loiter,
            mo_shot,
        } options_t;
    };

    struct ActionSpeed {
        uint8_t speed;  //0=cruise

        static inline uint16_t psize()
        {
            return sizeof(uint8_t);
        }
        inline void read(XbusStreamReader *s)
        {
            *s >> speed;
        }
        inline void write(XbusStreamWriter *s) const
        {
            *s << speed;
        }
    };

    struct ActionPoi {
        uint8_t poi;    //linked POI [1...n]

        static inline uint16_t psize()
        {
            return sizeof(uint8_t);
        }
        inline void read(XbusStreamReader *s)
        {
            *s >> poi;
        }
        inline void write(XbusStreamWriter *s) const
        {
            *s << poi;
        }
    };

    struct ActionScr {
        typedef std::array<char,16> scr_t; //public func @name
        scr_t scr;

        static inline uint16_t psize()
        {
            return sizeof(scr_t);
        }
        inline void read(XbusStreamReader *s)
        {
            *s >> scr;
        }
        inline void write(XbusStreamWriter *s) const
        {
            *s << scr;
        }
    };

    struct ActionLoiter {
        int16_t     turnR;
        uint8_t     loops;      //loops to loiter
        uint16_t    timeS;      //time to loiter

        static inline uint16_t psize()
        {
            return sizeof(int16_t)+sizeof(uint8_t)+sizeof(uint16_t);
        }
        inline void read(XbusStreamReader *s)
        {
            *s >> turnR;
            *s >> loops;
            *s >> timeS;
        }
        inline void write(XbusStreamWriter *s) const
        {
            *s << turnR;
            *s << loops;
            *s << timeS;
        }
    };

    struct ActionShot {
        uint16_t    dist;      //distance for series
        uint8_t     opt;       //0=single,1=start,2=stop

        static inline uint16_t psize()
        {
            return sizeof(uint16_t);
        }
        inline void read(XbusStreamReader *s)
        {
            uint16_t v;
            *s >> v;
            dist=v&0x0FFF;
            opt=v>>12;
        }
        inline void write(XbusStreamWriter *s) const
        {
            uint16_t v=(dist&0x0FFF)|((opt<<12)&0xF000);
            *s << v;
        }
    };

} // namespace
} // namespace
