#ifndef LOGFILE_H
#define LOGFILE_H


#include <fstream>
#include "ext/gsl-lite.h"
#include "gnss_packet.h"


class Logfile
{
public:
  Logfile(const std::string& filename);
  Logfile(const Logfile&) = delete;
  Logfile& operator=(const Logfile&) = delete;

  bool is_open() const { return fs_.is_open(); }

  template<typename T> void write(gsl::not_null<const gnss::PacketHeader*> header,
                                  const T* data,
                                  uint64_t receive_time);

private:
  void write(gsl::not_null<const gnss::LocationPacket*> data);

  std::ofstream fs_;
};


template<typename T> void Logfile::write(
    gsl::not_null<const gnss::PacketHeader*> header,
    const T* data,
    uint64_t receive_time)
{
  // Negative delay instead of an overflow when receive < transmit
  int64_t transmit_delay = receive_time - header->transmit_time;
  
  fs_ << header->packet_type << ','
      << receive_time << ','
      << header->transmit_time << ','
      << transmit_delay << ','
      << header->transmit_system_delay << ','
      << header->transmit_counter << ','
      << header->device_id << ',';

  write(data);

  fs_ << '\n';
}


#endif  // LOGFILE_H