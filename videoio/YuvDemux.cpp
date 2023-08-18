//
// Created by caizc on 6/15/22.
//

#include "../ThreadRunner.h"
#include "../xlog/xlog.h"
#include "YuvDemux.h"

namespace enflame {
YuvDemux::YuvDemux(const std::string &name) : Demux(name) {}

bool YuvDemux::initDemux(VideoCfgPtr cfg) {
  m_width = cfg->width;
  m_height = cfg->height;
  m_url = cfg->url;
  if (m_url == "fake.yuv") m_fake = true;
  return true;
}
bool YuvDemux::openDemux() {
  if (!m_fake) {
    m_f = fopen(m_url.c_str(), "rb");
    if (!m_f) {
      LOG_INFO("FOPEN failed.");
      return false;
    }
    LOG_INFO("open file : {}", m_url);
  } else {
    LOG_INFO("generate fake yuv");
  }
  return true;
}
bool YuvDemux::destory() {
  if (m_f) {
    fclose(m_f);
    m_f = nullptr;
    LOG_INFO("close file : {}", m_url);
  }
  return true;
}

bool YuvDemux::demux(VideoFramePtr out) {
  if (m_eof) return false;

  out->linesize[0] = m_width;
  out->linesize[1] = m_width / 2;
  out->linesize[2] = m_width / 2;

  size_t size[3];
  size[0] = m_width * m_height;
  size[1] = size[0] / 2;
  size[2] = size[0] / 2;

  for (int i = 0; i < 3; ++i) {
    out->data[i] = (uint8_t *)malloc(size[i]);
  }

  out->width = m_width;
  out->height = m_height;
  out->format = EnflamePixFormat ::PIX_YUV420;
  out->time_base = EN_TIME_BASE;
  out->pts = m_pts;
  m_pts += 33;

  if (!m_f) {
    int x, y, i;

    if (m_frameIdx >= (1 * 10)) {
      m_eof = true;
      return false;
    }
    i = m_frameIdx++;
    /* Y */
    for (y = 0; y < m_height; y++) {
      for (x = 0; x < m_width; x++) {
        if (x < m_width / 2 && y < m_height / 2)
          out->data[0][y * out->linesize[0] + x] = 0;
        else
          out->data[0][y * out->linesize[0] + x] = x + y + i * 3;
      }
    }

    /* Cb and Cr */
    for (y = 0; y < m_height / 2; y++) {
      for (x = 0; x < m_width / 2; x++) {
        if (x < m_width / 4 && y < m_height / 4) {
          out->data[1][y * out->linesize[1] + x] = 255;
          out->data[2][y * out->linesize[2] + x] = 128;
        } else {
          out->data[1][y * out->linesize[1] + x] = 128 + y + i * 2;
          out->data[2][y * out->linesize[2] + x] = 64 + x + i * 5;
        }
      }
    }
    // ThreadRunner::msleep(10);
  } else {
    uint8_t *avY = out->data[0];
    uint8_t *avU = out->data[1];
    uint8_t *avV = out->data[2];

    uint32_t pitchY = out->linesize[0];
    uint32_t pitchU = out->linesize[1];
    uint32_t pitchV = out->linesize[2];

    int ret = -1;
    for (uint32_t i = 0; i < m_height; i++) {
      if (fread(avY, pitchY, 1, m_f)) {
        avY += pitchY;
      } else {
        m_eof = true;
        return false;
      }
    }

    for (uint32_t i = 0; i < m_height / 2; i++) {
      if (fread(avU, pitchU, 1, m_f)) {
        avU += pitchU;
      } else {
        m_eof = true;
        return false;
      }
    }

    for (uint32_t i = 0; i < m_height / 2; i++) {
      if (fread(avV, pitchV, 1, m_f)) {
        avV += pitchV;
      } else {
        m_eof = true;
        return false;
      }
    }
    m_frameIdx++;
    ThreadRunner::msleep(40);
  }

  return true;
}
}  // namespace enflame