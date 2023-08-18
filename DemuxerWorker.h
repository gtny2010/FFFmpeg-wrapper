//
// Created by caizc on 6/14/22.
//

#ifndef TESTDAMEON_DEMUXERWORKER_H
#define TESTDAMEON_DEMUXERWORKER_H
#include <atomic>

#include "FpsGauge.h"
#include "SafeQueue.h"
#include "ThreadRunner.h"
#include "videoio/Demux.h"

namespace enflame {
class DemuxerWorker : public ThreadRunner {
 public:
  using DemuxerWorkerPtr = std::shared_ptr<DemuxerWorker>;
  explicit DemuxerWorker(DemuxPtr p);
  static DemuxerWorkerPtr create(DemuxPtr p) { return std::make_shared<DemuxerWorker>(p); }

  int64_t getIndex() { return m_demuxer->getIndex(); }
  void setUrl(const std::string &url) { m_url = url; }
  void setW(int w) { m_width = w; }
  void setH(int h) { m_height = h; }

  int getWidth() {
    while (!m_opened) ThreadRunner::msleep(5);
    return m_demuxer->getWidth();
  }
  int getHeight() {
    while (!m_opened) ThreadRunner::msleep(5);
    return m_demuxer->gerHeight();
  }
  EnflameCodecID getCocId() {
    while (!m_opened) ThreadRunner::msleep(5);
    return m_demuxer->getCodecId();
  }
  void *getStream() {
    while (!m_opened) ThreadRunner::msleep(5);
    return m_demuxer->getStream();
  }

  bool fetch(JobPkgPtr &baseptr);
  template <typename T>
  bool fetchT(std::shared_ptr<T> &out) {
    JobPkgPtr ptr;
    if (fetch(ptr)) {
      out = std::static_pointer_cast<T>(ptr);
      xassert(ptr.get());
      return true;
    } else {
      return false;
    }
  }

 protected:
  bool prepareToRun() final;
  void run() final;

 public:
  SafeQueue m_outCell{"demuxer-queue"};

 private:
  std::string m_url;
  int m_width{0};
  int m_height{0};
  bool m_opened{false};
  DemuxPtr m_demuxer{nullptr};
  FpsGauge m_fps{"demuxer-fps"};
  std::atomic<uint64_t> m_xLastFetchedFrmId{0};
};
using DemuxerWorkerPtr = std::shared_ptr<DemuxerWorker>;
}  // namespace enflame

#endif  // TESTDAMEON_DEMUXERWORKER_H
