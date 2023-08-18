#include <iostream>

#include "Allsurvey.h"
#include "BmpSurvey.h"
#include "DemuxerWorker.h"
#include "H264Survey.h"
#include "MuxWorker.h"
#include "Survey.h"
#include "YuvSurvey.h"
#include "videoio/FFmDecoder.h"
#include "videoio/FFmDemux.h"
#include "videoio/FFmEncoder.h"
#include "videoio/FFmMux.h"
#include "videoio/H264Mux.h"
#include "videoio/Utils.h"
#include "videoio/YuvMux.h"
#include "xlog/xlog.h"

using namespace enflame;

void test() {
  //  std::string inUrl = "/home/caizc/Videos/carflow.mp4";
  std::string inUrl = "/home/zhencheng.cai/Videos/Japan-street.264";
  std::string outUrl = "rtmp://127.0.0.1:1935/mytv/mystream";
  // std::string outUrl = "/home/caizc/Videos/test1.flv";

  VideoCfgPtr inCfg = std::make_shared<VideoCfg>();
  VideoCfgPtr outCfg = std::make_shared<VideoCfg>();
  VideoCfgPtr yuvCfg = std::make_shared<VideoCfg>();
  VideoCfgPtr h264Cfg = std::make_shared<VideoCfg>();
  VideoCodecParPtr decCfg = std::make_shared<VideoCodecPar>();
  VideoCodecParPtr encCfg = std::make_shared<VideoCodecPar>();

  FFmDemuxPtr demuxer = FFmDemux::create("demuxer");
  FFmMuxPtr muxer = FFmMux::create("muxer");
  FFmEncoderPtr encoder = FFmEncoder::create("encoder");
  FFmDecoderPtr decoder = FFmDecoder::create("decoder");
  YuvMuxPtr yuvWriter = YuvMux::create("yuver");
  H264MuxPtr h264Writer = H264Mux::create("h264writer");

  inCfg->url = inUrl;
  outCfg->url = outUrl;

  demuxer->initDemux(inCfg);
  demuxer->openDemux();

  decCfg->m_width = demuxer->getWidth();
  decCfg->m_height = demuxer->gerHeight();
  decCfg->m_codecName = "h264dec_omx";  //"h264";
  decCfg->codecParm = ((AVStream*)(demuxer->getStream()))->codecpar;
  decoder->setCodecPar(decCfg);
  decoder->init();
  decoder->open();

  yuvCfg->width = decCfg->m_width;
  yuvCfg->height = decCfg->m_height;
  yuvCfg->url = "/home/caizc/Videos/test.yuv";
  yuvWriter->initMux(yuvCfg);
  yuvWriter->openMux();

  encCfg->m_width = decCfg->m_width;
  encCfg->m_height = decCfg->m_height;
  encCfg->m_codecName = "libx264";
  encCfg->format = EnflamePixFormat ::PIX_YUV420;
  encoder->setCodecPar(encCfg);
  encoder->init();
  encoder->open();

  outCfg->width = demuxer->getWidth();
  outCfg->height = demuxer->gerHeight();
  outCfg->cId = demuxer->getCodecId();
  muxer->initMux(outCfg);
  muxer->setInStream((AVStream*)(demuxer->getStream()));
  //  muxer->setCodecCtx((AVCodecContext*)encoder->getCodecPtr()->codecParm);
  muxer->openMux();

  h264Cfg->url = "/home/caizc/Videos/h264.264";
  h264Writer->initMux(h264Cfg);
  h264Writer->openMux();

  VideoPacketPtr pkt = std::make_shared<VideoPacket>();
  VideoFramePtrArr frames;
  VideoPacketPtrArr pkts;
  int i = 0;
  while (!demuxer->getEof()) {
    frames.clear();
    pkts.clear();
    demuxer->demux(pkt);
    // muxer->mux(pkt);

    decoder->decode(pkt, frames);
    for (int i = 0; i < frames.size(); ++i) {
      // yuvWriter->mux(frames[i]);
      encoder->encode(frames[i], pkts);
      if (!pkts.empty()) {
        for (int j = 0; j < pkts.size(); ++j) {
          muxer->mux(pkts[j]);
          // h264Writer->mux(pkts[j]);
        }
        clearupPkts(pkts);
      }
    }

    clearupFrames(frames);
  }

  demuxer->destory();
  muxer->destory();
  decoder->destory();
  encoder->destory();
  h264Writer->destory();
  yuvWriter->destory();
}

int main() {
  // test();
  std::string inUrl = "rtsp://172.17.0.21:554/1920-1080-30.264";
  std::string inUrl1 = "/workspace/videos/testvideos/zzsin_352×288_30fps_60s.mp4";
  std::string outUrl = "rtmp://localhost:1935/myapp/mystream";
  std::string yuvUrl0 = "/workspace/FFmCodecTest/build/bin/carflow.yuv";
  std::string yuvUrl = "/workspace/FFmCodecTest/build/bin/enflame.yuv";
  std::string h264Url = "/workspace/FFmCodecTest/build/bin/enflame.264";

  // N0.1 this is a base sample for survey
  // auto survey = Survey ::create("survey");
  // survey->setUrl(inUrl, outUrl);
  // survey->startRun();

  // N0.2  this sample is for testing decode
  // auto yuv_survey = YuvSurvey ::create("YUV-survey");
  // yuv_survey->setUrl(inUrl1, yuvUrl);
  // yuv_survey->startRun();

  //  N0.3  this sample is for testing encode
  // auto h264_survey = H264Survey::create("H264-survey");
  // h264_survey->setUrl(yuvUrl, h264Url);
  // h264_survey->setUrl("fake.yuv", h264Url);
  // h264_survey->setW(1920);  // 176
  // h264_survey->setH(1080);  // 144
  // h264_survey->startRun();

  // N0.4 this sample is for decode and encode
  // auto all_survey = AllSurvey::create("All-survey");
  // all_survey->setUrl(inUrl1, outUrl);
  // all_survey->startRun();

  // N0.5 this sample is for decode and encode
  auto ppm_survey = BMPSurvey::create("PPm-survey");
  ppm_survey->setUrl("fake.yuv", yuvUrl);  // yuvUrl h264Url
  SwscaleCfg cfg;
  cfg.width = 32768;   // 640 8192x4320 32768*32768
  cfg.height = 17280;  // 360
  cfg.rewidth = 32768;
  cfg.reheight = 17280;
  cfg.pix = EnflamePixFormat::PIX_YUV420;
  cfg.repix = EnflamePixFormat::PIX_BGR24;
  ppm_survey->setParam(&cfg);
  ppm_survey->startRun();

  while (true) {
    ThreadRunner::msleep(5);
  }

  LOG_INFO("Exit process.");
  return 0;
}
