#pragma once
#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

class XbusNodeFilePayload
{
public:

    // Node file read-write
    enum {
        size_file_block = 256,
    };

    typedef struct{
      uint32_t      start_address;
      uint32_t      size;        // available size in bytes
      uint8_t       xor_crc;     // all file data XORed

      static inline uint16_t psize()
      {
          return sizeof(uint32_t)
                  + sizeof(uint32_t)
                  + sizeof(uint8_t);
      }
      void read(XbusStreamReader *s)
      {
          *s >> start_address;
          *s >> size;
          *s >> xor_crc;
      }
      void write(XbusStreamWriter *s) const
      {
          *s << start_address;
          *s << size;
          *s << xor_crc;
      }
    } file_t;

    typedef struct{
      uint32_t      start_address;
      uint16_t      data_size;

      static inline uint16_t psize()
      {
          return sizeof(uint32_t)
                  + sizeof(uint16_t);
      }
      void read(XbusStreamReader *s)
      {
          *s >> start_address;
          *s >> data_size;
      }
      void write(XbusStreamWriter *s) const
      {
          *s << start_address;
          *s << data_size;
      }
    } file_data_hdr_t;

    /*typedef struct{
      file_data_hdr_t  data_hdr;
      file_buf_t       data;

      static inline uint16_t psize()
      {
          return file_data_hdr_t::psize()
                  + sizeof(data);
      }
      void read(XbusStreamReader *s)
      {
          data_hdr.read(s);
          *s >> data;
      }
      void write(XbusStreamWriter *s) const
      {
          data_hdr.write(s);
          *s << data;
      }
    } file_data_t;*/

};

