//
// Created by caizc on 6/14/22.
//

#include "MuxWorker.h"
namespace enflame {
MuxWorker::MuxWorker(MuxPtr p) : ThreadRunner("Muxer"), m_muxer(p) {
  m_inCell.configDrop(false);
  m_inCell.configLength(100);
}

bool MuxWorker::add(JobPkgPtr &baseptr) {
  m_inCell.add(baseptr);
  return true;
}

bool MuxWorker::prepareToRun() {
  auto outCfg = std::make_shared<VideoCfg>();
  outCfg->url = m_url;

  // don't set
  //  outCfg->width = 0;
  //  outCfg->height = 0;
  //  outCfg->cId = 0;

  if (!m_muxer->initMux(outCfg)) {
    LOG_ERROR("init mux err.");
    return false;
  }

  // before open you should set stream or ctx()
  if (!m_muxer->openMux()) {
    LOG_ERROR("open mux err.");
    return false;
  }
  m_fps.restart();
  return true;
}

void MuxWorker::run() {
  while (!needToStop()) {
    if (std::string::npos == m_url.rfind(".yuv")) {
      JobPkgPacketPtr jobPkt;
      if (m_inCell.fetchT(jobPkt)) {
        if (m_muxer->mux(jobPkt->data)) {
          m_fps.tick();
          m_fps.tock();
          m_fps.debugPrint(10s);
        }
      }
    } else {
      JobPkgFramePtr jobPkt;
      if (m_inCell.fetchT(jobPkt)) {
        if (m_muxer->mux(jobPkt->data)) {
          m_fps.tick();
          m_fps.tock();
          m_fps.debugPrint(10s);
        }
      }
    }
    msleep(1);
  }  // while

  m_muxer->destory();
  m_inCell.clear();
  LOG_INFO("Muxer wrk exit.");
}
}  // namespace enflame