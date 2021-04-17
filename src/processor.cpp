#include <string>
#include <vector>
#include <numeric>
#include <unistd.h>

#include "processor.h"
#include "linux_parser.h"

using namespace std;

// Return the aggregate CPU utilization
float Processor::Utilization() {
  float utilization = 0;

  long active = LinuxParser::ActiveJiffies();
  long idle = LinuxParser::IdleJiffies();
  long total = active + idle;

  long prev_active = sample_prev_.at(0);
  long prev_idle = sample_prev_.at(1);
  long prev_total = prev_active + prev_idle;

  if(total > prev_total) {
    long delta_total_time = (total - prev_total);
    long delta_idle_time = (idle - prev_idle);
    utilization = (delta_total_time - delta_idle_time)
                  / static_cast<float>(delta_total_time);
  }

  sample_prev_.at(0) = active;
  sample_prev_.at(1) = idle;
  
  return utilization;
}
