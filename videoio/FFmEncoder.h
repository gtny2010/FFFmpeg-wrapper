//
// Created by caizc on 6/9/22.
//

#ifndef TESTDAMEON_FFMENCODER_H
#define TESTDAMEON_FFMENCODER_H

#include "Codec.h"
#include "FFmUtils.h"

namespace enflame {
class FFmEncoder : public Codec {
 public:
  using FFmEncoderPtr = std::shared_ptr<FFmEncoder>;
  explicit FFmEncoder(const std::string &name);
  ~FFmEncoder() override;
  static FFmEncoderPtr create(const std::string &name) {
    return std::make_shared<FFmEncoder>(name);
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
  void printErrMsg(int ret, const std::string &funName);

 private:
  AVCodecContext *m_encCtx{nullptr};
  AVFrame *m_frame{nullptr};
  AVPacket *m_pkt{nullptr};
  const AVCodec *m_enc{nullptr};

  constexpr static int ERR_BUF_LEN = 256;
  char *m_errBuf{nullptr};
  uint64_t m_pts{0};
  bool m_sendEnd{false};
};
using FFmEncoderPtr = std::shared_ptr<FFmEncoder>;
}  // namespace enflame

#endif  // TESTDAMEON_FFMENCODER_H
