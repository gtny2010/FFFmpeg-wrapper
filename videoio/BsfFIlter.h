//
// Created by caizc on 6/17/22.
//

#ifndef TESTDAMEON_BSFFILTER_H
#define TESTDAMEON_BSFFILTER_H
#include <memory>

#include "FFmUtils.h"
#include "Filter.h"

namespace enflame {
class BsfFilter : public Filter {
 public:
  using BsfFilterPtr = std::shared_ptr<BsfFilter>;
  explicit BsfFilter(const std::string &name);
  ~BsfFilter() override;
  static BsfFilterPtr create(const std::string &name) { return std::make_shared<BsfFilter>(name); }

  bool init() final;
  bool open() final;
  bool destory() final;
  bool filter(VideoPacketPtr in, VideoPacketPtr &out) final;
  bool filter(VideoFramePtr in, VideoFramePtr &out) final;
  void setparam(void *p) final { m_codecParam = (AVCodecParameters *)p; }

 private:
  void printErrMsg(int ret, const std::string &funName);

 private:
  AVBSFContext *m_bsfCtx{nullptr};
  AVCodecID m_codecId{AV_CODEC_ID_H264};
  constexpr static int ERR_BUF_LEN = 256;
  char *m_errBuf{nullptr};

  AVCodecContext *m_codecCtx{nullptr};
  AVCodecParameters *m_codecParam{nullptr};
};
using BsfFilterPtr = std::shared_ptr<BsfFilter>;
}  // namespace enflame

#endif  // TESTDAMEON_BSFFILTER_H
