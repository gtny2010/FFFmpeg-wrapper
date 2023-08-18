#include "BmpSurvey.h"
#include "videoio/FFmUtils.h"
#include "videoio/Utils.h"

// https :  // blog.csdn.net/summer_9527/article/details/86636889
namespace enflame {
BMPSurvey::BMPSurvey(const std::string &name) : Survey(name) {}

bool BMPSurvey::prepareToRun() {
  m_captureWrk = createYuvDemuxerWrk();
  m_captureWrk->setUrl(m_inUrl);
  m_captureWrk->setH(m_height);
  m_captureWrk->setW(m_width);
  m_captureWrk->startRun();

  m_swsPtr = SwscaleFilter::create("rgb-covert");
  m_swsPtr->setparam((void *)(m_swsCfg.get()));
  m_swsPtr->init();
  m_swsPtr->open();

  m_swsPtr2 = SwscaleFilter::create("yuv-covert");
  m_swsCfg2 = std::make_shared<SwscaleCfg>(*m_swsCfg);
  m_swsCfg2->pix = EnflamePixFormat::PIX_BGR24;
  m_swsCfg2->repix = EnflamePixFormat::PIX_YUV420;
  m_swsPtr2->setparam((void *)(m_swsCfg2.get()));
  m_swsPtr2->init();
  m_swsPtr2->open();

  if (m_outUrl.find(".264") != std::string::npos)
    m_writerWrk = createH264MuxerWrk();
  else
    m_writerWrk = createYuvMuxerWrk();

  m_encoder = FFmEncoder::create("ffm-encoder");

  m_writerWrk->setUrl(m_outUrl);
  m_captureWrk->startRun();

  m_encCfg = std::make_shared<VideoCodecPar>();
  m_encCfg->m_width = m_width;
  m_encCfg->m_height = m_height;
  m_encCfg->m_codecName = "libx264";  // libx264
  m_encCfg->format = EnflamePixFormat ::PIX_YUV420;
  m_encCfg->codecParm = nullptr;
  m_encoder->setCodecPar(m_encCfg);
  m_encoder->init();
  m_encoder->open();

  m_writerWrk->startRun();

  return true;
}

void BMPSurvey::run() {
  int index = 0;
  while (!needToStop()) {
    if (m_encoder->getEof()) {
      auto idx1 = m_captureWrk->getIndex();
      auto idx2 = m_writerWrk->getIndex();
      if (idx1 == idx2) break;
    }

    auto idx1 = m_captureWrk->getIndex();
    auto idx2 = m_encoder->getSendIndex();
    if (idx2 >= idx1 && idx1 != 0) {
      LOG_INFO("in idx == out idx, end survey.");
      m_encoder->setEOf(true);
    }

    JobPkgFramePtr jobPkt = nullptr;
    auto size = m_captureWrk->m_outCell.size();
    bool ret = m_captureWrk->fetchT(jobPkt);

    VideoFramePtr rgbframe = nullptr;
    if (jobPkt && jobPkt->data) {
      m_swsPtr->filter(jobPkt->data, rgbframe);
    }

    VideoFramePtr outframe = nullptr;
    picText(rgbframe, outframe);
    VideoFramePtr yuvframe = nullptr;
    m_swsPtr2->filter(outframe, yuvframe);

    // if (ret)
    //   yuvframe = jobPkt->data;
    // else
    //   continue;

    if (m_outUrl.find(".yuv") != std::string::npos && yuvframe) {
      auto wPkt = std::make_shared<JobPkgFrame>();
      JobPkgPtr tmp = nullptr;
      wPkt->data = yuvframe;
      wPkt->timeStamp = FpsGauge::currentTimePoint();
      tmp = wPkt;
      m_writerWrk->add(tmp);
    } else {
      VideoPacketPtrArr pkts;
      m_encoder->encode(yuvframe, pkts);
      for (int i = 0; i < pkts.size(); ++i) {
        auto wPkt = std::make_shared<JobPkgPacket>();
        JobPkgPtr tmp = nullptr;
        wPkt->data = pkts[i];
        wPkt->timeStamp = FpsGauge::currentTimePoint();
        tmp = wPkt;
        m_writerWrk->add(tmp);
      }
    }

    m_fps.tick();
    m_fps.tock();
    m_fps.debugPrint(10s);

    ThreadRunner::msleep(1);
  }

  m_swsPtr->destory();
  m_writerWrk->stopAndWait();
  m_captureWrk->stopAndWait();
  LOG_INFO("Stop BMP-survey wrk.");
}

bool BMPSurvey::picText(VideoFramePtr in, VideoFramePtr &out) {
  if (!in || !in->data[0]) return false;

  out = std::make_shared<VideoFrame>();
  out->copyTags(in);
  int w = in->width;
  int h = in->height;
  cv::Mat mat;
  mat.create(cv::Size(w, h), CV_8UC3);
  size_t size = in->linesize[0] * h;
  memcpy(mat.data, in->data[0], size);
  char text[256] = {0};
  sprintf(text, "www.enflame-tech.cn");
  putText(mat, text, cv::Point(10, h / 8), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 1,
          4);
  sprintf(text, "video %dx%d", w, h);
  putText(mat, text, cv::Point(10, h / 4), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 1,
          4);
  sprintf(text, "%d/60", m_frameIndex++);
  putText(mat, text, cv::Point(10, (h / 4 + h / 8)), cv::FONT_HERSHEY_SIMPLEX, 1,
          cv::Scalar(0, 0, 255), 1, 4);
  // cv::imwrite("./3nflame.jpg", mat);
  bool cont = false;
  cont = mat.isContinuous();
  auto total = mat.total();
  auto channel = mat.channels();
  auto rgbSize = total * channel;
  out->data[0] = (uint8_t *)malloc(rgbSize);
  memset(out->data[0], 0, rgbSize);
  memcpy(out->data[0], mat.data, rgbSize);
  out->linesize[0] = in->linesize[0];
  out->format = EnflamePixFormat::PIX_BGR24;

  // cv::Mat mattest;
  // mattest.create(cv::Size(w, h), CV_8UC3);
  // memcpy(mattest.data, out->data[0], size);
  // cv::imwrite("./4nflame.jpg", mattest);
  return true;
}

void BMPSurvey::ppmSave(VideoFramePtr in, int width, int height, int index) {
  if (!in) return;
  AVFrame *pframe{nullptr};
  pframe = av_frame_alloc();

  int ret = -1;
  pframe->time_base = {1, in->time_base};
  pframe->pts = in->pts;
  pframe->width = in->width;
  pframe->height = in->height;
  pframe->format = pixTras(in->format);

  for (int i = 0; i < MAX_NUM; ++i) {
    pframe->linesize[i] = in->linesize[i];
    pframe->data[i] = in->data[i];
    in->data[i] = nullptr;
    in->linesize[i] = 0;
  }

  FILE *pFile;
  char szFilename[32];
  int y;

  // Open file
  sprintf(szFilename, "./frame%d.ppm", index);  //文件名
  pFile = fopen(szFilename, "wb");

  if (pFile == nullptr) return;

  // Write header
  fprintf(pFile, "P6 %d %d 255", width, height);

  // Write pixel data
  for (y = 0; y < height; y++) {
    if (pframe->data[0]) {
      fwrite(pframe->data[0] + y * pframe->linesize[0], 1, width * 3, pFile);
    }
  }
  fclose(pFile);
  // Close file

  //原文链接：https://blog.csdn.net/qq_37933895/article/details/100015875
  // BITMAPFILEHEADER bmpheader;
  // BITMAPINFO bmpinfo;
  // int y = 0;
  // FILE *pFile;
  // char szFilename[32];

  // unsigned char *y_buf = pFrame->data[0];
  // sprintf(szFilename, "frame%d.bmp", iFrame);
  // pFile = fopen(szFilename, "wb");

  // bmpheader.bfReserved1 = 0;
  // bmpheader.bfReserved2 = 0;
  // bmpheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
  // bmpheader.bfSize = bmpheader.bfOffBits + width * height * 24 / 8;

  // bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  // bmpinfo.bmiHeader.biWidth = width;
  // bmpinfo.bmiHeader.biHeight = -height;
  // bmpinfo.bmiHeader.biPlanes = 1;
  // bmpinfo.bmiHeader.biBitCount = 24;
  // bmpinfo.bmiHeader.biCompression = BI_RGB;
  // bmpinfo.bmiHeader.biSizeImage = 0;
  // bmpinfo.bmiHeader.biXPelsPerMeter = 100;
  // bmpinfo.bmiHeader.biYPelsPerMeter = 100;
  // bmpinfo.bmiHeader.biClrUsed = 0;
  // bmpinfo.bmiHeader.biClrImportant = 0;

  // fwrite(&bmpheader, sizeof(BITMAPFILEHEADER), 1, pFile);
  // fwrite(&bmpinfo.bmiHeader, sizeof(BITMAPINFOHEADER), 1, pFile);

  // fwrite(pFrame->data[0], width * height * 24 / 8, 1, pFile);

  // /*for(y=0; y<height; y++)
  // {
  //     fwrite(pFrame->data[0] + y*pFrame->linesize[0], 1, width*3, pFile);
  // }*/

  // // fflush(pFile);
  // fclose(pFile);
}
}  // namespace enflame