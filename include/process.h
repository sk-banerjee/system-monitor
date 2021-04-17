#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  int Pid();
  std::string User();
  std::string Command();
  float CpuUtilization();
  std::string Ram();
  long int UpTime();
  bool operator<(Process const& a) const;
  bool operator>(Process const& a) const ;
  void ProcessLastTimeSample(long& sched_tck, long& seconds_up) {
    sched_tck = prev_sched_tck_;
    seconds_up = prev_seconds_up_;
  }

  Process(int pid, long sched_tck, long uptime, float utilization)
          : pid_(pid), prev_sched_tck_(sched_tck), prev_seconds_up_(uptime), 
            cpu_utilization_(utilization) {}

 private:
   int pid_;
   long prev_sched_tck_ {0};
   long prev_seconds_up_ {0};
   float cpu_utilization_ {0};
};

#endif