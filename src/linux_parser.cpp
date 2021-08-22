#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

float LinuxParser::MemoryUtilization() {
  string line;
  string key;
  float value;
  float MemTotal;
  float MemFree;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal:") {
          MemTotal = value;
        } else if (key == "MemFree:") {
          MemFree = value;
        }
      }
    }
  }
  return (MemTotal - MemFree) / MemTotal;
}

long LinuxParser::UpTime() {
  string line;
  long up_time{};
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> up_time;
    return up_time;
  }
  return up_time;
}

long LinuxParser::Jiffies() { return ActiveJiffies() + IdleJiffies(); }

long LinuxParser::ActiveJiffies(int pid) {
  string line;
  string value;
  int i = 1;
  float utime, stime, cutime, cstime;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      if (i == 14)
        utime = stof(value);
      else if (i == 15)
        stime = stof(value);
      else if (i == 16)
        cutime = stof(value);
      else if (i == 17)
        cstime = stof(value);
      i++;
    }
    return utime + stime + cutime + cstime;
  }
  return 0;
}

long LinuxParser::ActiveJiffies() {
  string line;
  string value;
  std::vector<int> cpuValues;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      if (value != "cpu") cpuValues.emplace_back(stof(value));
    }
    return cpuValues[kUser_] + cpuValues[kNice_] + cpuValues[kSystem_] +
           cpuValues[kIRQ_] + cpuValues[kSoftIRQ_] + cpuValues[kSteal_];
  }
  return 0;
}

long LinuxParser::IdleJiffies() {
  string line;
  string value;
  std::vector<int> cpuValues;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      if (value != "cpu") cpuValues.push_back(stof(value));
    }
    return cpuValues[kIdle_] + cpuValues[kIOwait_];
  }
  return 0;
}

float LinuxParser::CpuUtilization() {
  float active = ActiveJiffies();
  float idle = IdleJiffies();
  if (active + idle > 0.0f)
    return active / (active + idle);
  else
    return 0.0f;
}

int LinuxParser::TotalProcesses() {
  string line;
  string key;
  int value{};
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") return value;
      }
    }
  }
  return value;
}

int LinuxParser::RunningProcesses() {
  string line;
  string key;
  int value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") return value;
      }
    }
  }
  return value;
}

string LinuxParser::Command(int pid) {
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
  }
  return line;
}

string LinuxParser::Ram(int pid) {
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        // I have used VmRSS instead of VmSize to get physycal memory instead of virtual memory. 
        if (key == "VmRSS:") return to_string((int)(stof(value) * 0.001));
      }
    }
  }
  return "0";
}

string LinuxParser::Uid(int pid) {
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "Uid:") return value;
      }
    }
  }
  return "Uid";
}

string LinuxParser::User(int pid) {
  string line, key, x, value;
  string uid = Uid(pid);
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> x >> value) {
        if (value == uid) {
          return key;
        }
      }
    }
    filestream.close();
  }
  return key;
}

long LinuxParser::UpTime(int pid) {
  string line;
  string value;
  int i = 1;
  float starttime;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      if (i == 22) starttime = stof(value);
      i++;
    }
    return UpTime() - (starttime / sysconf(_SC_CLK_TCK));
  }
  return 0;
}
