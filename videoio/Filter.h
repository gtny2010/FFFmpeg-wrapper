//
// Created by caizc on 6/17/22.
//

#ifndef TESTDAMEON_FILTER_H
#define TESTDAMEON_FILTER_H
#include <atomic>
#include <memory>
#include <vector>

#include "../InstanceName.h"
#include "Types.h"
namespace enflame {
class Filter : public InstanceName {
 public:
  using FilterPtr = std::shared_ptr<Filter>;
  explicit Filter(const std::string &name);

  virtual bool init() = 0;
  virtual bool open() = 0;
  virtual bool destory() = 0;
  virtual bool filter(VideoPacketPtr in, VideoPacketPtr &out) = 0;
  virtual bool filter(VideoFramePtr in, VideoFramePtr &out) = 0;
  virtual void setparam(void *p) = 0;

 protected:
  StateType m_stateFlag{StateType::INVALID};
};
}  // namespace enflame

#endif  // TESTDAMEON_FILTER_H
