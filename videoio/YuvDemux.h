//
// Created by caizc on 6/15/22.
//

#ifndef TESTDAMEON_YUVDEMUX_H
#define TESTDAMEON_YUVDEMUX_H
#include <iostream>

#include "Demux.h"

namespace enflame {
class YuvDemux : public Demux {
 public:
  using YuvDemuxPtr = std::shared_ptr<YuvDemux>;
  explicit YuvDemux(const std::string &name);
  static YuvDemuxPtr create(const std::string &name) { return std::make_shared<YuvDemux>(name); }

  bool initDemux(VideoCfgPtr cfg) final;
  bool openDemux() final;
  bool destory() final;
  bool demux(VideoFramePtr out) final;
  bool demux(VideoPacketPtr out) final { return true; }

  void *getStream() final { return nullptr; }

 private:
  FILE *m_f{nullptr};
  int64_t m_pts{0};
  bool m_fake{false};
};
using YuvDemuxPtr = std::shared_ptr<YuvDemux>;
}  // namespace enflame

#endif  // TESTDAMEON_YUVDEMUX_H
