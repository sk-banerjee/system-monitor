#include <string>
#include <sstream>
#include <iomanip>

#include "format.h"

using std::string;

// Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
  std::stringstream time_stream;
  long hh = seconds / 3600;
  long mm = (seconds % 3600) / 60;
  long ss = (seconds % 3600) % 60;

  time_stream << std::setw(2) << std::setfill('0') << hh
              << std::setw(1) << ':'
              << std::setw(2) << std::setfill('0') << mm
              << std::setw(1) << ':'
              << std::setw(2) << std::setfill('0') << ss;
  return time_stream.str();
}