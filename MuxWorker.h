//
// Created by caizc on 6/14/22.
//

#ifndef TESTDAMEON_MUXWORKER_H
#define TESTDAMEON_MUXWORKER_H
#include "FpsGauge.h"
#include "SafeQueue.h"
#include "ThreadRunner.h"
#include "videoio/Mux.h"

namespace enflame {
class MuxWorker : public ThreadRunner {
 public:
  using MuxWorkerPtr = std::shared_ptr<MuxWorker>;
  MuxWorker(MuxPtr p);
  static MuxWorkerPtr create(MuxPtr p) { return std::make_shared<MuxWorker>(p); }
  void setUrl(const std::string &url) { m_url = url; }

  void setCodecCtx(void *ctx) { m_muxer->setCodecCtx(ctx); }
  void setInStream(void *stream) { m_muxer->setInStream(stream); }
  bool add(JobPkgPtr &baseptr);
  int64_t getIndex() { return m_muxer->getIndex(); }
  void setEOf(bool eof) { m_muxer->setEof(eof); }
  bool getEof() { return m_muxer->getEof(); }

 protected:
  bool prepareToRun() final;
  void run() final;

 private:
  std::string m_url;
  MuxPtr m_muxer{nullptr};
  SafeQueue m_inCell{"muxer-queue"};
  FpsGauge m_fps{"muxer-fps"};
};
using MuxWorkerPtr = std::shared_ptr<MuxWorker>;
}  // namespace enflame

#endif  // TESTDAMEON_MUXWORKER_H
