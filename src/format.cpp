#include <string>

#include "format.h"

using std::string;

string Format::ElapsedTime(long seconds) {
  std::string result;
  // compute h, m, s
  std::string h = std::to_string(seconds / 3600);
  std::string m = std::to_string((seconds % 3600) / 60);
  std::string s = std::to_string(seconds % 60);
  // add leading zero if needed
  std::string hh = std::string(2 - h.length(), '0') + h;
  std::string mm = std::string(2 - m.length(), '0') + m;
  std::string ss = std::string(2 - s.length(), '0') + s;
  // return mm:ss if hh is 00
  if (hh.compare("00") != 0) {
    result = hh + ':' + mm + ":" + ss;
  } else {
    result = mm + ":" + ss;
  }
  return result;
}