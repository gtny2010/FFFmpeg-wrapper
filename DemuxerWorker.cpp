//
// Created by caizc on 6/14/22.
//

#include "DemuxerWorker.h"

namespace enflame {
DemuxerWorker::DemuxerWorker(DemuxPtr p) : ThreadRunner("demuxer"), m_demuxer(p) {
  m_outCell.configDrop(false);
  m_outCell.configLength(100);
}

bool DemuxerWorker::fetch(JobPkgPtr &baseptr) {
  auto result = m_outCell.fetch(baseptr);
  if (result) {
    m_xLastFetchedFrmId.store(baseptr->frameId, std::memory_order_release);
  }
  return result;
}

bool DemuxerWorker::prepareToRun() {
  auto inCfg = std::make_shared<VideoCfg>();
  inCfg->url = m_url;
  inCfg->height = m_height;
  inCfg->width = m_width;

  if (!m_demuxer->initDemux(inCfg)) {
    LOG_ERROR("init demuxer err.");
    return false;
  }

  if (!m_demuxer->openDemux()) {
    LOG_ERROR("open demuxer err.");
    return false;
  }

  m_fps.restart();
  return true;
}

void DemuxerWorker::run() {
  m_opened = true;
  ino64_t idx = 0;
  while (!needToStop()) {
    if (m_demuxer->getEof()) msleep(5);
    if (std::string::npos == m_url.rfind(".yuv")) {
      auto pkt = std::make_shared<VideoPacket>();
      if (m_demuxer->demux(pkt)) {
        JobPkgPacketPtr jobPkt = std::make_shared<JobPkgPacket>();
        jobPkt->data = pkt;
        jobPkt->timeStamp = FpsGauge::currentTimePoint();
        m_outCell.add(jobPkt);
        idx++;
        m_fps.tick();
        m_fps.tock();
        m_fps.debugPrint(10s);
      }
    } else {
      auto pkt = std::make_shared<VideoFrame>();
      if (m_demuxer->demux(pkt)) {
        JobPkgFramePtr jobPkt = std::make_shared<JobPkgFrame>();
        jobPkt->data = pkt;
        jobPkt->timeStamp = FpsGauge::currentTimePoint();
        m_outCell.add(jobPkt);
        idx++;
        m_fps.tick();
        m_fps.tock();
        m_fps.debugPrint(10s);
      }
    }
    // not good , will drop some pkts.
    if (m_demuxer->getEof() && idx >= m_demuxer->getIndex()) {
      // LOG_INFO("end of stream.:{}", m_demuxer->getIndex());
      // break;
    }
    msleep(1);
  }  // while

  m_demuxer->destory();
  m_outCell.clear();
  m_opened = false;
  LOG_INFO("Demuxer wrk exit.");
}

}  // namespace enflame