//
// Created by caizc on 6/7/22.
//

#ifndef TESTDAMEON_MUX_H
#define TESTDAMEON_MUX_H

#include <atomic>
#include <memory>
#include <vector>

#include "../InstanceName.h"
#include "Types.h"

namespace enflame {

class Mux : InstanceName {
 public:
  using MuxPtr = std::shared_ptr<Mux>;

  Mux(const std::string &name);

  virtual bool initMux(VideoCfgPtr cfg) = 0;
  virtual bool openMux() = 0;
  virtual bool destory() = 0;
  virtual bool mux(VideoPacketPtr in) = 0;
  virtual bool mux(VideoFramePtr in) = 0;
  void setEof(bool eof) { m_eof = eof; }
  bool getEof() { return m_eof; }

  virtual void setCodecCtx(void *ctx){};
  virtual void setInStream(void *stream){};
  int64_t getIndex() { return m_frameIdx; }

 protected:
  StateType m_stateFlag{StateType::INVALID};
  std::string m_url{""};
  int m_width{0};
  int m_height{0};
  EnflameCodecID m_cId{EnflameCodecID::CODEC_ID_NONE};
  std::atomic_bool m_eof{false};
  int64_t m_frameIdx{0};
};
using MuxPtr = std::shared_ptr<Mux>;
}  // namespace enflame

#endif  // TESTDAMEON_MUX_H
