//
// Created by caizc on 6/13/22.
//

#include "../xlog/xlog.h"
#include "H264Mux.h"

namespace enflame {
H264Mux::H264Mux(const std::string &name) : Mux(name) {}

bool H264Mux::initMux(VideoCfgPtr cfg) {
  m_url = cfg->url;
  return true;
}

bool H264Mux::openMux() {
  m_f = fopen(m_url.c_str(), "wb");
  if (!m_f) {
    LOG_INFO("FOPEN failed.");
    return false;
  }
  return true;
}

bool H264Mux::destory() {
  if (m_f) {
    fclose(m_f);
    LOG_INFO("close file : {}", m_url);
  }
  return true;
}
bool H264Mux::mux(VideoPacketPtr in) {
  fwrite(in->data, in->size, 1, m_f);
  fflush(m_f);
  m_frameIdx++;
  return true;
}
}  // namespace enflame