//
// Created by caizc on 6/15/22.
//

#ifndef TESTDAMEON_ALLSURVEY_H
#define TESTDAMEON_ALLSURVEY_H
#include "Survey.h"
#include "videoio/BsfFIlter.h"
namespace enflame {
class AllSurvey : public Survey {
 public:
  using AllsurveyPtr = std::shared_ptr<AllSurvey>;
  explicit AllSurvey(const std::string &name);
  static AllsurveyPtr create(const std::string &name) { return std::make_shared<AllSurvey>(name); }

 protected:
  bool prepareToRun() final;
  void run() final;

 private:
  FFmDecoderPtr m_decoder{nullptr};
  FFmEncoderPtr m_encoder{nullptr};
  VideoCodecParPtr m_decCfg{nullptr};
  VideoCodecParPtr m_encCfg{nullptr};
  BsfFilterPtr m_bsf{nullptr};
};
using AllsurveyPtr = std::shared_ptr<AllSurvey>;
}  // namespace enflame

#endif  // TESTDAMEON_ALLSURVEY_H
