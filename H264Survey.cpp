//
// Created by caizc on 6/15/22.
//

#include "H264Survey.h"

namespace enflame {
H264Survey::H264Survey(const std::string &name) : Survey(name) {}

bool H264Survey::prepareToRun() {
  m_captureWrk = createYuvDemuxerWrk();
  m_writerWrk = createH264MuxerWrk();
  m_encoder = FFmEncoder::create("ffm-encoder");

  m_encCfg = std::make_shared<VideoCodecPar>();

  m_captureWrk->setUrl(m_inUrl);
  m_captureWrk->setH(m_height);
  m_captureWrk->setW(m_width);

  m_writerWrk->setUrl(m_outUrl);
  m_captureWrk->startRun();

  m_encCfg->m_width = m_width;
  m_encCfg->m_height = m_height;
  m_encCfg->m_codecName = "libx264";
  m_encCfg->format = EnflamePixFormat ::PIX_YUV420;
  m_encoder->setCodecPar(m_encCfg);
  m_encoder->init();
  m_encoder->open();

  m_writerWrk->startRun();

  return true;
}
void H264Survey::run() {
  while (!needToStop()) {
    if (!m_captureWrk->isRunning()) break;

    JobPkgFramePtr jobPkt;
    if (m_captureWrk->fetchT(jobPkt)) {
      VideoPacketPtrArr pkts;
      m_encoder->encode(jobPkt->data, pkts);
      for (int i = 0; i < pkts.size(); ++i) {
        auto wPkt = std::make_shared<JobPkgPacket>();
        JobPkgPtr tmp;
        wPkt->data = pkts[i];
        wPkt->timeStamp = FpsGauge::currentTimePoint();
        tmp = wPkt;
        m_writerWrk->add(tmp);
      }
      m_fps.tick();
      m_fps.tock();
      m_fps.debugPrint(10s);
    }

    ThreadRunner::msleep(1);
  }
  m_encoder->destory();
  m_writerWrk->stopAndWait();
  m_captureWrk->stopAndWait();
  LOG_INFO("Stop YUV-survey wrk.");
}
}  // namespace enflame