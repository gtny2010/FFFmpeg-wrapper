//
// Created by caizc on 6/9/22.
//

#ifndef TESTDAMEON_FFMDECODER_H
#define TESTDAMEON_FFMDECODER_H

#include "Codec.h"
#include "FFmUtils.h"

namespace enflame {
class FFmDecoder : public Codec {
 public:
  using FFmDecoderPtr = std::shared_ptr<FFmDecoder>;
  explicit FFmDecoder(const std::string &name);
  ~FFmDecoder() override;
  static FFmDecoderPtr create(const std::string &name) {
    return std::make_shared<FFmDecoder>(name);
  }

  bool init() final;
  bool open() final;
  bool destory() final;
  bool decode(VideoPacketPtr in, VideoFramePtrArr &out) final;

  // for decoder ,ecode() depricaated
  bool encode(VideoFramePtr in, VideoPacketPtrArr &out) final {
    // do nothing
    return true;
  }

 private:
  void printErrMsg(int ret, const std::string &funName);

 private:
  AVCodecContext *m_decCtx{nullptr};
  AVFrame *m_frame{nullptr};
  AVPacket *m_pkt{nullptr};
  const AVCodec *m_dec{nullptr};
  AVCodecParameters *m_params{nullptr};
  constexpr static int ERR_BUF_LEN = 256;
  char *m_errBuf{nullptr};
  VideoFramePtrArr m_inner;
  int64_t m_pts{0};
};
using FFmDecoderPtr = std::shared_ptr<FFmDecoder>;
}  // namespace enflame

#endif  // TESTDAMEON_FFMDECODER_H
