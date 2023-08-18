//
// Created by caizc on 6/14/22.
//
#include "Survey.h"

#include "videoio/Utils.h"

namespace enflame {

Survey::Survey(const std::string &name) : ThreadRunner(name) {}

void Survey::setUrl(const std::string &in, const std::string &out) {
  m_inUrl = in;
  m_outUrl = out;
}

bool Survey::prepareToRun() {
  m_captureWrk = createFFmDemuxerWrk();
  m_writerWrk = createFFmMuxerWrk();

  m_captureWrk->setUrl(m_inUrl);
  m_writerWrk->setUrl(m_outUrl);
  m_captureWrk->startRun();

  m_writerWrk->setInStream(m_captureWrk->getStream());
  m_writerWrk->startRun();
  m_fps.restart();
  return true;
}

void Survey::run() {
  while (!needToStop()) {
    JobPkgPacketPtr jobPkt;
    if (m_captureWrk->fetchT(jobPkt)) {
      JobPkgPtr tmp = jobPkt;
      m_writerWrk->add(tmp);
      m_fps.tick();
      m_fps.tock();
      m_fps.debugPrint(10s);
    }

    ThreadRunner::msleep(1);
  }

  m_writerWrk->stopAndWait();
  m_captureWrk->stopAndWait();
}
}  // namespace enflame