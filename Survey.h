//
// Created by caizc on 6/14/22.
//

#ifndef TESTDAMEON_SURVEY_H
#define TESTDAMEON_SURVEY_H

#include <memory>

#include "Factory.h"
#include "videoio/BsfFIlter.h"
#include "videoio/FFmDecoder.h"
#include "videoio/FFmEncoder.h"
#include "xlog/xlog.h"

namespace enflame {
class Survey : public ThreadRunner {
 public:
  using SurveyPtr = std::shared_ptr<Survey>;
  explicit Survey(const std::string &name);
  static SurveyPtr create(const std::string &name) { return std::make_shared<Survey>(name); }
  void setUrl(const std::string &in, const std::string &out);
  void setW(int w) { m_width = w; }
  void setH(int h) { m_height = h; }

 protected:
  bool prepareToRun() override;
  void run() override;
  virtual void setParam(void *p) {}

 protected:
  bool m_needCodec{true};
  std::string m_inUrl{"rtsp://10.33.61.22:8554/300.264"};
  std::string m_outUrl{"rtmp://localhost:1935/mytv/mystream"};
  DemuxerWorkerPtr m_captureWrk{nullptr};
  MuxWorkerPtr m_writerWrk{nullptr};
  FpsGauge m_fps{"survey"};
  int m_width{0};
  int m_height{0};
};
using SurveyPtr = std::shared_ptr<Survey>;
}  // namespace enflame

#endif  // TESTDAMEON_SURVEY_H
