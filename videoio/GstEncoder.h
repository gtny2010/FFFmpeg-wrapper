//
// Created by caizc on 6/9/22.
//

#ifndef TESTDAMEON_GSTENCODER_H
#define TESTDAMEON_GSTENCODER_H
#include "Codec.h"

namespace enflame {
class GstEncoder : public Codec {
 public:
  using GstEncoderPtr = std::shared_ptr<GstEncoder>;
  explicit GstEncoder(const std::string &name);
  ~GstEncoder() override;
  static GstEncoderPtr create(const std::string &name) {
    return std::make_shared<GstEncoder>(name);
  }

  bool init() final;
  bool open() final;
  bool destory() final;
  bool encode(VideoFramePtr in, VideoPacketPtrArr &out) final;

  // for encoder ,ecode() depricaated
  bool decode(VideoPacketPtr in, VideoFramePtrArr &out) final {
    // do nothing
    return true;
  }

 private:
  uint64_t m_pts{0};
  bool m_sendEnd{false};
};
using GstEncoderPtr = std::shared_ptr<GstEncoder>;
}  // namespace enflame

#endif TESTDAMEON_GSTENCODER_H