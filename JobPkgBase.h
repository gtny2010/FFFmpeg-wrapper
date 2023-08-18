//
// Created by caizc on 6/14/22.
//

#ifndef TESTDAMEON_JOBPKGBASE_H
#define TESTDAMEON_JOBPKGBASE_H
#include <chrono>
#include <memory>

#include "videoio/Types.h"

namespace enflame {
class JobPkgBase {
 public:
  using NanoSeconds = std::chrono::nanoseconds;
  using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock, NanoSeconds>;

  JobPkgBase() = default;
  JobPkgBase(JobPkgBase &) = default;
  JobPkgBase(const JobPkgBase &) = default;
  JobPkgBase &operator=(JobPkgBase &) = default;
  JobPkgBase &operator=(const JobPkgBase &) = default;
  ~JobPkgBase() = default;

  JobPkgBase &copyTags(const JobPkgBase &in) {
    // do copy
    return *this;
  }

  JobPkgBase &copyTags(const std::shared_ptr<JobPkgBase> &in) {
    // do copy
    return *this;
  }

  TimePoint timeStamp;
  uint64_t syncId{0};
  uint64_t frameId{0};
};

template <typename T>
struct JobPkgT : public JobPkgBase {
  T data;

  JobPkgT<T> &deepCopy(const JobPkgT<T> &src) {
    JobPkgBase::copyTags(static_cast<JobPkgBase>(src));
    data = src.data;
    return *this;
  }
};
using JobPkgPtr = std::shared_ptr<JobPkgBase>;

template <typename SubType>
inline JobPkgPtr cast_to_job_base(SubType job) {
  return std::static_pointer_cast<JobPkgBase>(job);
}

template <typename SubType>
inline std::shared_ptr<SubType> cast_to_job_of(JobPkgPtr job) {
  return std::static_pointer_cast<SubType>(job);
}

using JobPkgFrame = JobPkgT<VideoFramePtr>;
using JobPkgPacket = JobPkgT<VideoPacketPtr>;

using JobPkgFramePtr = std::shared_ptr<JobPkgFrame>;
using JobPkgPacketPtr = std::shared_ptr<JobPkgPacket>;

}  // namespace enflame

#endif  // TESTDAMEON_JOBPKGBASE_H
