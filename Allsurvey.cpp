//
// Created by caizc on 6/15/22.
//

#include "Allsurvey.h"
#include "videoio/Utils.h"
namespace enflame {
AllSurvey::AllSurvey(const std::string &name) : Survey(name) {}
bool AllSurvey::prepareToRun() {
  m_captureWrk = createFFmDemuxerWrk();
  m_writerWrk = createFFmMuxerWrk();
  m_decoder = FFmDecoder::create("ffm-decoder");
  m_encoder = FFmEncoder::create("ffm-encoder");
  m_bsf = BsfFilter::create("bsf-filter");

  m_decCfg = std::make_shared<VideoCodecPar>();
  m_encCfg = std::make_shared<VideoCodecPar>();

  m_captureWrk->setUrl(m_inUrl);
  m_writerWrk->setUrl(m_outUrl);
  m_captureWrk->startRun();

  m_decCfg->m_width = m_captureWrk->getWidth();
  m_decCfg->m_height = m_captureWrk->getHeight();
  m_decCfg->m_codecName = "h264dec_omx";  //"h264";h264dec_omx
  m_decCfg->codecParm = ((AVStream *)(m_captureWrk->getStream()))->codecpar;
  m_decoder->setCodecPar(m_decCfg);
  m_decoder->init();
  m_decoder->open();

  m_bsf->setparam(((AVStream *)(m_captureWrk->getStream()))->codecpar);
  m_bsf->init();
  m_bsf->open();

  m_encCfg->m_width = m_decCfg->m_width;
  m_encCfg->m_height = m_decCfg->m_height;
  m_encCfg->m_codecName = "libx264";
  m_encCfg->format = EnflamePixFormat ::PIX_YUV420;
  m_encoder->setCodecPar(m_encCfg);
  m_encoder->init();
  m_encoder->open();

  m_writerWrk->setCodecCtx((AVCodecContext *)m_encoder->getCodecPtr()->codecParm);
  m_writerWrk->startRun();
  m_fps.restart();
  return true;
}

void AllSurvey::run() {
  while (!needToStop()) {
    if (!m_captureWrk->isRunning()) break;
    JobPkgPacketPtr jobPkt;
    VideoPacketPtr p = std::make_shared<VideoPacket>();
    if (m_captureWrk->fetchT(jobPkt)) {
      // m_bsf->filter(jobPkt->data, p);
      VideoFramePtrArr frames;
      m_decoder->decode(jobPkt->data, frames);
      for (int i = 0; i < frames.size(); ++i) {
        VideoPacketPtrArr pkts;
        m_encoder->encode(frames[i], pkts);
        if (!pkts.empty()) {
          for (int j = 0; j < pkts.size(); ++j) {
            auto wPkt = std::make_shared<JobPkgPacket>();
            JobPkgPtr tmp;
            wPkt->data = pkts[j];
            tmp = wPkt;
            m_writerWrk->add(tmp);
          }  // for
        }
      }  // for

      m_fps.tick();
      m_fps.tock();
      m_fps.debugPrint(10s);
    }

    ThreadRunner::msleep(1);
  }  // while

  m_writerWrk->stopAndWait();
  m_captureWrk->stopAndWait();
  m_decoder->destory();
  m_encoder->destory();
}
}  // namespace enflame