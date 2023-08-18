//
// Created by caizc on 6/7/22.
//

#ifndef TESTDAMEON_TYPES_H
#define TESTDAMEON_TYPES_H

#include <atomic>
#include <memory>
#include <vector>

namespace enflame {
#define EN_TIME_BASE 1000
#define EN_TIME_BASE_Q \
  { 1, 1000 }

#define MAX_NUM 8
enum class StateType { INVALID, OPENED, CLOSED };
enum class EnflameCodecID { CODEC_ID_NONE, CODEC_ID_H264, CODEC_ID_MPEG4 };
enum class EnflamePixFormat { PIX_NONE, PIX_YUV420, PIX_BGR24 };

class VideoCfg {
 public:
  std::string url{""};
  int width{0};
  int height{0};
  EnflameCodecID cId{EnflameCodecID::CODEC_ID_NONE};
  EnflamePixFormat format;
  std::shared_ptr<void> reserved{nullptr};
};
using VideoCfgPtr = std::shared_ptr<VideoCfg>;

class VideoPacket {
 public:
  size_t size{0};
  uint8_t *data{nullptr};
  int64_t pts{-1};
  int64_t dts{-1};
  int64_t duration{0};
  int time_base{1000};
  bool keyFrame{false};
  EnflameCodecID cId{EnflameCodecID::CODEC_ID_NONE};
  int width{0};
  int height{0};

  ~VideoPacket() {
    if (data) {
      free(data);
      data = nullptr;
    }
    size = 0;
  }
};
using VideoPacketPtr = std::shared_ptr<VideoPacket>;
using VideoPacketPtrArr = std::vector<VideoPacketPtr>;

class VideoFrame {
 public:
  using VideoFramePtr = std::shared_ptr<VideoFrame>;
  uint8_t *data[MAX_NUM] = {nullptr};
  int linesize[MAX_NUM] = {0};
  int64_t pts{-1};
  int64_t dur{0};
  int width{0};
  int height{0};
  int time_base{0};
  EnflamePixFormat format;

  ~VideoFrame() {
    for (int i = 0; i < MAX_NUM; ++i) {
      if (data[i]) {
        free(data[i]);
        data[i] = nullptr;
        // printf("~VideoFrame,free data\n");
      }
      linesize[i] = 0;
    }
    // printf("~VideoFrame\n");
  }

  void copyTags(VideoFramePtr src) {
    width = src->width;
    height = src->height;
    dur = src->dur;
    pts = src->pts;
    time_base = src->time_base;
    format = src->format;
  }
};
using VideoFramePtr = std::shared_ptr<VideoFrame>;
using VideoFramePtrArr = std::vector<VideoFramePtr>;

}  // namespace enflame
#endif  // TESTDAMEON_TYPES_H
