//
// Created by caizc on 6/7/22.
//

#ifndef TESTDAMEON_DEMUX_H
#define TESTDAMEON_DEMUX_H
#include <atomic>
#include <memory>
#include <vector>

#include "../InstanceName.h"
#include "Types.h"

namespace enflame {

class Demux : InstanceName {
 public:
  using DemuxPtr = std::shared_ptr<Demux>;

  Demux(const std::string &name);

  virtual bool initDemux(VideoCfgPtr cfg) = 0;
  virtual bool openDemux() = 0;
  virtual bool destory() = 0;
  virtual bool demux(VideoPacketPtr out) = 0;
  virtual bool demux(VideoFramePtr out) = 0;
  virtual void *getStream() = 0;
  int64_t getIndex() { return m_frameIdx; }
  std::string getUrl() const { return m_url; }
  int getWidth() const { return m_width; }
  int gerHeight() const { return m_height; }
  EnflameCodecID getCodecId() const { return m_codecId; }
  void setEof(bool eof) { m_eof = eof; }
  bool getEof() const { return m_eof; }

 protected:
  StateType m_stateFlag{StateType::INVALID};
  std::string m_url{""};
  std::atomic_bool m_eof{false};
  int m_width{0};
  int m_height{0};
  EnflameCodecID m_codecId{EnflameCodecID::CODEC_ID_NONE};
  EnflamePixFormat m_pix{EnflamePixFormat::PIX_NONE};
  std::atomic_int64_t m_frameIdx{0};
};
using DemuxPtr = std::shared_ptr<Demux>;
}  // namespace enflame

#endif  // TESTDAMEON_DEMUX_H
