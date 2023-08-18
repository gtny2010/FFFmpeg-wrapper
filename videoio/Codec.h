//
// Created by caizc on 6/7/22.
//

#ifndef TESTDAMEON_CODEC_H
#define TESTDAMEON_CODEC_H
#include <atomic>
#include <memory>
#include <vector>

#include "../InstanceName.h"
#include "Types.h"

namespace enflame {
class VideoCodecPar {
 public:
  std::string m_codecName{"unknow"};
  int m_width{0};
  int m_height{0};
  int m_gpu{-1};
  void *codecParm{nullptr};
  EnflamePixFormat format;
  EnflameCodecID cId{EnflameCodecID::CODEC_ID_NONE};
};
using VideoCodecParPtr = std::shared_ptr<VideoCodecPar>;

class Codec : InstanceName {
 public:
  using CodecPtr = std::shared_ptr<Codec>;
  Codec(const std::string &name);

  virtual bool init() = 0;
  virtual bool open() = 0;
  virtual bool destory() = 0;
  virtual bool decode(VideoPacketPtr in, VideoFramePtrArr &out) = 0;
  virtual bool encode(VideoFramePtr in, VideoPacketPtrArr &out) = 0;

  void setCodecPar(VideoCodecParPtr ptr) { m_codecParPtr = ptr; }
  VideoCodecParPtr getCodecPtr() const { return m_codecParPtr; }
  int64_t getIndex() { return m_frameIdx; }
  int64_t getSendIndex() { return m_sendFrameIdx; }
  void setEOf(bool eof) { m_eof = eof; }
  bool getEof() { return m_eof; }

 protected:
  std::atomic_bool m_eof{false};
  StateType m_stateFlag{StateType::INVALID};
  VideoCodecParPtr m_codecParPtr{nullptr};
  std::atomic_int64_t m_frameIdx{0};
  std::atomic_int64_t m_sendFrameIdx{0};
};
using CodecPtr = std::shared_ptr<Codec>;
}  // namespace enflame

#endif  // TESTDAMEON_CODEC_H
