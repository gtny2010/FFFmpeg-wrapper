//
// Created by caizc on 6/8/22.
//

#ifndef TESTDAMEON_FFMMUX_H
#define TESTDAMEON_FFMMUX_H

#include "FFmUtils.h"
#include "Mux.h"

namespace enflame {
class FFmMux : public Mux {
 public:
  using FFmMuxPtr = std::shared_ptr<FFmMux>;
  explicit FFmMux(const std::string &name);
  ~FFmMux() override;
  static FFmMuxPtr create(const std::string &name) { return std::make_shared<FFmMux>(name); }

  bool initMux(VideoCfgPtr cfg) final;
  bool openMux() final;
  bool destory() final;
  bool mux(VideoPacketPtr in) final;
  bool mux(VideoFramePtr in) final {
    // do something
    return true;
  }

  void setCodecCtx(void *ctx) final { m_codeCtx = (AVCodecContext *)ctx; }
  void setInStream(void *stream) final { m_inStream = (AVStream *)stream; }

 private:
  void printErrMsg(int ret, const std::string &funName);

 private:
  AVFormatContext *m_avfmtCtx{nullptr};
  AVCodecContext *m_codeCtx{nullptr};
  AVPacket *m_pkt{nullptr};
  AVStream *m_outStream{nullptr};
  AVStream *m_inStream{nullptr};

  constexpr static int ERR_BUF_LEN = 256;
  char *m_errBuf{nullptr};
  uint64_t m_pts{0};
};
using FFmMuxPtr = std::shared_ptr<FFmMux>;
}  // namespace enflame

#endif  // TESTDAMEON_FFMMUX_H
