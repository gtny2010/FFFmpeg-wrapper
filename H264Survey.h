//
// Created by caizc on 6/15/22.
//

#ifndef TESTDAMEON_H264SURVEY_H
#define TESTDAMEON_H264SURVEY_H

#include "Survey.h"
#include "videoio/FFmEncoder.h"
namespace enflame {
class H264Survey : public Survey {
 public:
  using H264SurveyPtr = std::shared_ptr<H264Survey>;
  explicit H264Survey(const std::string &name);
  static H264SurveyPtr create(const std::string &name) { return std::make_shared<H264Survey>(name); }

 protected:
  bool prepareToRun() final;
  void run() final;

 private:
  FFmEncoderPtr m_encoder{nullptr};
  VideoCodecParPtr m_encCfg{nullptr};
};
using H264SurveyPtr = std::shared_ptr<H264Survey>;
}  // namespace enflame

#endif  // TESTDAMEON_H264SURVEY_H
