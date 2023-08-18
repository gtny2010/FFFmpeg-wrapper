//
// Created by caizc on 6/30/22.
//

#ifndef TESTDAMEON_SWSCALEFILTER_H
#define TESTDAMEON_SWSCALEFILTER_H

#include "FFmUtils.h"
#include "Filter.h"

namespace enflame {
class SwscaleCfg {
 public:
  int width;
  int height;
  EnflamePixFormat pix;
  int rewidth;             // after resize
  int reheight;            // after resize
  EnflamePixFormat repix;  // after convert
};
using SwscaleCfgPtr = std::shared_ptr<SwscaleCfg>;

class SwscaleFilter : public Filter {
 public:
  using SwscaleFilterPtr = std::shared_ptr<SwscaleFilter>;
  explicit SwscaleFilter(const std::string &name);
  ~SwscaleFilter() override;
  static SwscaleFilterPtr create(const std::string &name) {
    return std::make_shared<SwscaleFilter>(name);
  }

  bool init() final;
  bool open() final;
  bool destory() final;
  bool filter(VideoFramePtr in, VideoFramePtr &out) final;
  bool filter(VideoPacketPtr in, VideoPacketPtr &out) final { return true; }
  void setparam(void *p) final {
    auto cfg = (SwscaleCfg *)p;
    m_swsCfg = std::make_shared<SwscaleCfg>(*cfg);
  };

 private:
  // AVPixelFormat pixTras(EnflamePixFormat color);
  // EnflamePixFormat pixTras2(AVPixelFormat color);
  void printErrMsg(int ret, const std::string &funName);

 private:
  AVFrame *m_RGB{nullptr};
  AVFrame *m_frame{nullptr};
  constexpr static int ERR_BUF_LEN = 256;
  char *m_errBuf{nullptr};

  SwscaleCfgPtr m_swsCfg{nullptr};
  SwsContext *m_imgConvertCtx{nullptr};
  uint8_t *m_buffer{nullptr};
  size_t m_numBytes{0};
};
using SwscaleFilterPtr = std::shared_ptr<SwscaleFilter>;
}  // namespace enflame
#endif