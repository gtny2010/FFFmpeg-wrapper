//
// Created by caizc on 6/30/22.
//

#ifndef TESTDAMEON_BMPSURVEY_H
#define TESTDAMEON_BMPSURVEY_H

#include "Survey.h"
#include "opencv2/opencv.hpp"
#include "videoio/SwscaleFilter.h"

namespace enflame {
class BMPSurvey : public Survey {
 public:
  using BMPSurveyPtr = std::shared_ptr<BMPSurvey>;
  explicit BMPSurvey(const std::string &name);

  static BMPSurveyPtr create(const std::string &name) { return std::make_shared<BMPSurvey>(name); }
  void setParam(void *p) final {
    auto e = (SwscaleCfg *)p;
    m_swsCfg = std::make_shared<SwscaleCfg>(*e);
    m_width = m_swsCfg->width;
    m_height = m_swsCfg->height;
  }

 protected:
  bool prepareToRun() final;
  void run() final;

 private:
  void ppmSave(VideoFramePtr in, int width, int height, int iFrame);
  bool picText(VideoFramePtr in, VideoFramePtr &out);

 private:
  SwscaleFilterPtr m_swsPtr{nullptr};
  SwscaleFilterPtr m_swsPtr2{nullptr};
  SwscaleCfgPtr m_swsCfg{nullptr};
  SwscaleCfgPtr m_swsCfg2{nullptr};
  int m_frameIndex{0};

  FFmEncoderPtr m_encoder{nullptr};
  VideoCodecParPtr m_encCfg{nullptr};
};
using BMPSurveyPtr = std::shared_ptr<BMPSurvey>;
}  // namespace enflame

#endif  // TESTDAMEON_ALLSURVEY_H