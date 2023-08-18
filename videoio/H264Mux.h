//
// Created by caizc on 6/13/22.
//

#ifndef TESTDAMEON_H264MUX_H
#define TESTDAMEON_H264MUX_H
#include <iostream>

#include "Mux.h"

namespace enflame {
class H264Mux : public Mux {
 public:
  using H264MuxPtr = std::shared_ptr<H264Mux>;
  H264Mux(const std::string &name);
  static H264MuxPtr create(const std::string &name) { return std::make_shared<H264Mux>(name); }
  bool initMux(VideoCfgPtr cfg) final;
  bool openMux() final;
  bool destory() final;
  bool mux(VideoPacketPtr in) final;
  bool mux(VideoFramePtr in) final {
    // do something
    return true;
  }

 private:
  FILE *m_f;
};
using H264MuxPtr = std::shared_ptr<H264Mux>;
}  // namespace enflame

#endif  // TESTDAMEON_H264MUX_H
