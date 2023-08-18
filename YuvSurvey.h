//
// Created by caizc on 6/15/22.
//

#ifndef TESTDAMEON_YUVSURVEY_H
#define TESTDAMEON_YUVSURVEY_H
#include "Survey.h"
#include "videoio/FFmDecoder.h"
namespace enflame {
class YuvSurvey : public Survey {
 public:
  using YuvSurveyPtr = std::shared_ptr<YuvSurvey>;
  explicit YuvSurvey(const std::string &name);
  static YuvSurveyPtr create(const std::string &name) { return std::make_shared<YuvSurvey>(name); }

 protected:
  bool prepareToRun() final;
  void run() final;

 private:
  FFmDecoderPtr m_decoder{nullptr};
  VideoCodecParPtr m_decCfg{nullptr};
};
using YuvSurveyPtr = std::shared_ptr<YuvSurvey>;
}  // namespace enflame

#endif  // TESTDAMEON_YUVSURVEY_H
