//
// Created by caizc on 6/7/22.
//

#ifndef TESTDAMEON_FFMDEMUX_H
#define TESTDAMEON_FFMDEMUX_H
#include "Demux.h"
#include "FFmUtils.h"

namespace enflame {
class FFmDemux : public Demux {
 public:
  using FFmDemuxPtr = std::shared_ptr<FFmDemux>;

  explicit FFmDemux(const std::string &name);
  ~FFmDemux() override;
  static FFmDemuxPtr create(const std::string &name) { return std::make_shared<FFmDemux>(name); }

  bool initDemux(VideoCfgPtr cfg) final;
  bool openDemux() final;
  bool destory() final;
  bool demux(VideoPacketPtr out) final;
  bool demux(VideoFramePtr out) final { return true; }

  void *getStream() final { return m_videoStream; }

 private:
  EnflameCodecID codeIDTras(AVCodecID ID);

  void printErrMsg(int ret, const std::string &funName);

 private:
  AVFormatContext *m_avformatCtx{nullptr};
  AVStream *m_videoStream{nullptr};
  AVPacket *m_pkt{nullptr};
  AVDictionary *m_dic{nullptr};
  int m_videoIndex{-1};

  bool m_firstPktFlag{false};
  int64_t m_startTime{0};
  int64_t m_firstPtsTime{0};

  constexpr static int ERR_BUF_LEN = 256;
  char *m_errBuf{nullptr};
};
using FFmDemuxPtr = std::shared_ptr<FFmDemux>;
}  // namespace enflame

#endif  // TESTDAMEON_FFMDEMUX_H
