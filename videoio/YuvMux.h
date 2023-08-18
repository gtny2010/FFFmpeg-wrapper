//
// Created by caizc on 6/10/22.
//

#ifndef TESTDAMEON_YUVMUX_H
#define TESTDAMEON_YUVMUX_H
#include <iostream>

#include "Mux.h"

namespace enflame {
class YuvMux : public Mux {
 public:
  using YuvMuxPtr = std::shared_ptr<YuvMux>;
  explicit YuvMux(const std::string &name);

  static YuvMuxPtr create(const std::string &name) { return std::make_shared<YuvMux>(name); }

  bool initMux(VideoCfgPtr cfg) final;
  bool openMux() final;
  bool destory() final;
  bool mux(VideoPacketPtr in) final {
    // do something
    return true;
  }
  bool mux(VideoFramePtr in) final;

 private:
  FILE *m_f;
};
using YuvMuxPtr = std::shared_ptr<YuvMux>;
}  // namespace enflame

#endif  // TESTDAMEON_YUVMUX_H
