//
// Created by caizc on 6/10/22.
//

#include "../xlog/xlog.h"
#include "YuvMux.h"

namespace enflame {
YuvMux::YuvMux(const std::string &name) : Mux(name) {}

bool YuvMux::initMux(VideoCfgPtr cfg) {
  m_width = cfg->width;
  m_height = cfg->height;
  m_url = cfg->url;
  return true;
}

bool YuvMux::openMux() {
  m_f = fopen(m_url.c_str(), "wb");
  if (!m_f) {
    LOG_INFO("FOPEN failed.");
    return false;
  }
  return true;
}

bool YuvMux::destory() {
  if (m_f) {
    fclose(m_f);
    LOG_INFO("close file : {}", m_url);
  }
  return true;
}

bool YuvMux::mux(VideoFramePtr in) {
  uint32_t pitchY = in->linesize[0];
  uint32_t pitchU = in->linesize[1];
  uint32_t pitchV = in->linesize[2];

  uint8_t *avY = in->data[0];
  uint8_t *avU = in->data[1];
  uint8_t *avV = in->data[2];

  for (uint32_t i = 0; i < in->height; i++) {
    fwrite(avY, in->width, 1, m_f);
    avY += pitchY;
  }

  for (uint32_t i = 0; i < in->height / 2; i++) {
    fwrite(avU, in->width / 2, 1, m_f);
    avU += pitchU;
  }

  for (uint32_t i = 0; i < in->height / 2; i++) {
    fwrite(avV, in->width / 2, 1, m_f);
    avV += pitchV;
  }
  fflush(m_f);
  m_frameIdx++;
  return true;
}

}  // namespace enflame