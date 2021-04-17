#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <sstream>
#include <numeric>
#include <thread>
#include <experimental/filesystem>  

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;
namespace fs = std::experimental::filesystem;

// An example of how to read data from the filesystem
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

// An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// return a vector of pids
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  fs::path proc_dir(kProcDirectory);
  for (const auto& entry : fs::directory_iterator(proc_dir)) {
    string fname(entry.path().filename().c_str());
    // Is this a directory?
    if (fs::is_directory(entry.status())) {
      // Is every character of the name a digit?
      if (std::all_of(fname.begin(), fname.end(), isdigit)) {
        int pid = std::stoi(fname);
        pids.push_back(pid);
      }
    }
  }
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  int total_memory;
  int free_memory;
  int records_to_read = 2;
  if (filestream.is_open()) {
    while (std::getline(filestream, line) && records_to_read) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal:") {
          std::stringstream result(value);
          result >> total_memory;
          records_to_read--;
        }
        if (key == "MemFree:") {
          std::stringstream result(value);
          result >> free_memory;
          records_to_read--;
        }
      }
    }
  }
  return (total_memory - free_memory) / static_cast<float>(total_memory);
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  string line;
  string time;
  long uptime = 0;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> time;
    std::stringstream result(time);
    result >> uptime;
  }
  return uptime;
}

int LinuxParser::NumCpu(){
  return std::thread::hardware_concurrency();
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return sysconf(_SC_CLK_TCK); }

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  vector<long> proc_stat = LinuxParser::CpuUtilization(pid);
  long active = proc_stat.at(0) + proc_stat.at(1);
  return active;
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  std::vector<std::string> proc_stat = LinuxParser::CpuUtilization();
  std::vector<long> samples;

  for(auto value: proc_stat) {
    samples.push_back(std::stoi(value));
  }

  long total = accumulate(samples.begin(), samples.begin()+7, 0);
  long idle =  accumulate(samples.begin()+3, samples.begin()+5, 0);

  return (total - idle);
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  std::vector<std::string> proc_stat = LinuxParser::CpuUtilization();
  std::vector<long> samples;

  for(auto value: proc_stat) {
    samples.push_back(std::stoi(value));
  }

  return accumulate(samples.begin()+3, samples.begin()+5, 0);
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  string line;
  string key;
  vector<string> values;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> key;
    string temp;
    while(linestream >> temp) {
       values.push_back(temp);
    }
  }
  return values;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line;
  string key;
  string value;
  int total_processes = 0;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") {
          std::stringstream result(value);
          result >> total_processes;
        }
      }
    }
  }
  return total_processes;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line;
  string key;
  string value;
  int running_processes = 0;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") {
          std::stringstream result(value);
          result >> running_processes;
        }
      }
    }
  }
  return running_processes;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
    string line{};
    string pid_dir(std::to_string(pid));
    std::ifstream filestream(kProcDirectory + pid_dir + kCmdlineFilename);
    if (filestream.is_open()) {
      std::getline(filestream, line);
    }
    return line;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string line{};
  string key{};
  string vm_size{};
  string unit;
  string pid_dir(std::to_string(pid));
  std::ifstream filestream(kProcDirectory + pid_dir + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> vm_size >> unit;
      if (key == "VmSize:") {
          if (unit == "kB")
            return std::to_string(std::stoi(vm_size) / 1024);
          else
            return vm_size;
      }
    }
  }
  return string("0");
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string line{};
  string key{};
  string value{};
  string pid_dir(std::to_string(pid));
  std::ifstream filestream(kProcDirectory + pid_dir + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "Uid:")
          return value;
      }
    }    
  }
  return string();
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string uid = LinuxParser::Uid(pid);
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    string line{};
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      string username;
      string password;
      string etc_uid;
      linestream >> username >> password >> etc_uid;
      if (uid == etc_uid)
        return username;
    }
  }
  return string();
}

// Read and return the uptime of a process in seconds
long LinuxParser::UpTime(int pid) {
  string line{};
  string value{};
  string pid_dir(std::to_string(pid));
  int position = 22;
  std::ifstream filestream(kProcDirectory + pid_dir + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    do {
      linestream >> value;
    } while (--position);
    long start_time_clk_ticks = std::stoi(value);
    long start_time_sec = start_time_clk_ticks / LinuxParser::Jiffies();
    return LinuxParser::UpTime() - start_time_sec;
  }
  return 0;
}

// Returns a vector of following values for process with pid
//  * [0] utime - Amount of time process has been scheduled in user mode
//  * [1] stime - Amount of time process has been scheduled in kernel mode
//  * [2] cutime - Amount of time process waited-for children have been
//                  scheduled in user mode
//  * [3] cstime - Amount of time process waited-for children have been
//                  scheduled in kernel mode
//  * [4] starttime - The time the process started after system boot.
vector<long> LinuxParser::CpuUtilization(int pid) {
  string line{};
  string value{};
  std::vector<long> process_stat_info(5, 0);
  string pid_dir(std::to_string(pid));
  int position = 1;
  int index = 0;
  std::ifstream filestream(kProcDirectory + pid_dir + kStatFilename);

  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    while(linestream >> value) {
      if((position >= 14 && position <= 17) || position == 22) {
        process_stat_info.at(index++) = std::stoi(value);
      }
      position++;
    }
  }

  return process_stat_info;
}
