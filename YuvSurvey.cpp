//
// Created by caizc on 6/15/22.
//

#include "YuvSurvey.h"
#include "videoio/Utils.h"
namespace enflame {
YuvSurvey::YuvSurvey(const std::string &name) : Survey(name) {}

bool YuvSurvey::prepareToRun() {
  m_captureWrk = createFFmDemuxerWrk();
  m_writerWrk = createYuvMuxerWrk();
  m_decoder = FFmDecoder::create("ffm-decoder");

  m_decCfg = std::make_shared<VideoCodecPar>();

  m_captureWrk->setUrl(m_inUrl);
  m_writerWrk->setUrl(m_outUrl);
  m_captureWrk->startRun();

  m_decCfg->m_width = m_captureWrk->getWidth();
  m_decCfg->m_height = m_captureWrk->getHeight();
  m_decCfg->m_codecName = "h264";  //"h264"; h264dec_omx
  m_decCfg->codecParm = ((AVStream *)(m_captureWrk->getStream()))->codecpar;
  m_decoder->setCodecPar(m_decCfg);
  m_decoder->init();
  m_decoder->open();

  m_writerWrk->startRun();

  return true;
}

void YuvSurvey::run() {
  while (!needToStop()) {
    if (!m_captureWrk->isRunning()) break;
    // VideoFramePtrArr frames;
    JobPkgPacketPtr jobPkt;
    if (m_captureWrk->fetchT(jobPkt)) {
      VideoFramePtrArr frames;
      // clearupFrames(frames);
      m_decoder->decode(jobPkt->data, frames);
      for (int i = 0; i < frames.size(); ++i) {
        auto wPkt = std::make_shared<JobPkgFrame>();
        JobPkgPtr tmp;
        wPkt->data = frames[i];
        wPkt->timeStamp = FpsGauge::currentTimePoint();
        tmp = wPkt;
        m_writerWrk->add(tmp);
        m_fps.tick();
        m_fps.tock();
        m_fps.debugPrint(10s);
      }
    }

    ThreadRunner::msleep(1);
  }
  m_decoder->destory();
  m_writerWrk->stopAndWait();
  m_captureWrk->stopAndWait();
  LOG_INFO("Stop YUV-survey wrk.");
}

}  // namespace enflame