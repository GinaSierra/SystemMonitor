#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"

using std::string;
using std::to_string;
using std::vector;

int Process::Pid() { return Pid_; }

float Process::CpuUtilization() {
  if (UpTime() > 0.0f)
    return ((LinuxParser::ActiveJiffies(Pid()) / sysconf(_SC_CLK_TCK)) /
                  UpTime());
  else
    return 0.0f;
}

string Process::Command() { return LinuxParser::Command(Pid()); }

string Process::Ram() { return LinuxParser::Ram(Pid()); }

string Process::User() { return LinuxParser::User(Pid()); }

long int Process::UpTime() { return LinuxParser::UpTime(Pid()); }

bool Process::operator<(Process& a) {
  return a.CpuUtilization() < this->CpuUtilization();
}