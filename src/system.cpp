#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>
#include <numeric>
#include <map>

#include "process.h"
#include "processor.h"
#include "system.h"
#include "linux_parser.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;
using std::map;

// Return the system's CPU
Processor& System::Cpu() { return cpu_; }

// Return a container composed of the system's processes
vector<Process>& System::Processes() {
  vector<int> pids = LinuxParser::Pids();
  map<int, vector<long>> processes_map;

  for(auto proc : processes_) { 
    vector<long> t(2, 0);
    proc.ProcessLastTimeSample(t[0], t[1]);
    processes_map[proc.Pid()]= t;
  }

  processes_.erase(processes_.begin(),  processes_.end());

  for(auto pid : pids) {
    vector<long> proc_stat = LinuxParser::CpuUtilization(pid);
    long sched_tck = LinuxParser::ActiveJiffies(pid);
    long seconds_up = LinuxParser::UpTime(pid);
    long prev_sched_tck = 0;
    long prev_seconds_up = 0;
    auto it = processes_map.find(pid);
  
    if(it != processes_map.end()) {
      prev_sched_tck = it->second.at(0);
      prev_seconds_up = it->second.at(1);
    }

    float delta_sched_sec = static_cast<float>(abs(sched_tck - prev_sched_tck))
                            / LinuxParser::Jiffies();
    long delta_up_time = (seconds_up - prev_seconds_up)?(seconds_up - prev_seconds_up):1;
    float utilization = delta_sched_sec / static_cast<float>(delta_up_time);
    Process process(pid, sched_tck, seconds_up, utilization);

    processes_.push_back(process);
  }
  std::sort(processes_.begin(), processes_.end(), std::greater<Process>());
  return processes_;
}

int System::NumCpu() { return LinuxParser::NumCpu(); }

// Return the system's kernel identifier (string)
std::string System::Kernel() { return kernel_; }

// Return the system's memory utilization
float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }

// Return the operating system name
std::string System::OperatingSystem() { return operating_system_; }

// Return the number of processes actively running on the system
int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }

// Return the total number of processes on the system
int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }

// Return the number of seconds since the system started running
long int System::UpTime() { return LinuxParser::UpTime(); }

System::System() {
  operating_system_ = LinuxParser::OperatingSystem();
  kernel_ = LinuxParser::Kernel();
}