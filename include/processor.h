#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <string>
#include <vector>

class Processor {
 public:
  float Utilization();
  Processor() {
    sample_prev_.push_back(0);
    sample_prev_.push_back(0);
  }

 private:
  std::vector<long> sample_prev_;
};

#endif