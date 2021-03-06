#include "nmea_reader.h"


#include <algorithm>
#include <functional>
#include <iostream>

#include "../util/util.h"


namespace
{


inline gsl::span<char> find_sentence(gsl::span<char> buffer)
{
  auto it = std::find_if(std::begin(buffer), std::end(buffer), [](char c) { return c == '\n'; });
  if (it != std::end(buffer))
    return buffer.first(std::distance(std::begin(buffer), it) + 1);  // + 1 to move past \n
  return gsl::span<char>{};
};


}  // namespace


pichi::NmeaReader::NmeaReader(const util::Timer& timer, std::condition_variable& data_ready,
    std::mutex& data_mutex, std::deque<NmeaSentence>& data)
    : timer_{timer}, data_ready_{data_ready}, data_mutex_{data_mutex}, data_{data}
{
}


pichi::NmeaReader::~NmeaReader()
{
}


void pichi::NmeaReader::start(const std::string& port, uint32_t rate)
{
  start_(port, rate);
}


void pichi::NmeaReader::reset()
{
  activity_counter_.store(0);
}


void pichi::NmeaReader::reset_buffer()
{
  buffer_end_ = std::begin(buffer_);
}


void pichi::NmeaReader::handle_read(gsl::span<char> received_data)
{
  auto read_time = timer_.now();

  auto buffer_begin = std::begin(buffer_);
  auto data_size = std::distance(buffer_begin, buffer_end_);
  data_size += received_data.size();

  if (static_cast<std::size_t>(data_size) > BUFFER_SIZE) {
    reset_buffer();
    data_size = received_data.size();
  }

  std::copy(std::begin(received_data), std::end(received_data), buffer_end_);
  std::advance(buffer_end_, received_data.size());
  auto pos = process_data(gsl::span<char>{buffer_}.first(data_size), read_time);

  // Remove processed data from buffer
  if (pos > 0) {
    int n = data_size - pos;
    std::copy_n(buffer_begin + pos, n, buffer_begin);
    buffer_end_ = buffer_begin + n;
  }
}


int pichi::NmeaReader::process_data(gsl::span<char> buffer, util::TimePoint read_time)
{
  std::size_t pos = 0;
  std::lock_guard<std::mutex> lock(data_mutex_);

  do {
    auto sentence = find_sentence(buffer.subspan(pos, buffer.size() - pos));
    if (!sentence.empty()) {
      std::string s{std::begin(sentence), std::end(sentence)};
      data_.emplace_back(read_time, nmea::deduce_sentence_type(s), std::move(s));
      pos += sentence.size();
    }
  } while (pos < buffer.size());

  // Prevent overflow due to no one consuming the sentences
  while (data_.size() > 30)
    data_.pop_front();

  if (!data_.empty()) {
    data_ready_.notify_one();
  }

  return pos;
}
