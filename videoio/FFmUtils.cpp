//
// Created by caizc on 6/9/22.
//

#include "FFmUtils.h"
namespace enflame {
AVPixelFormat pixTras(EnflamePixFormat pix) {
  switch (pix) {
    case EnflamePixFormat::PIX_YUV420:
      return AV_PIX_FMT_YUV420P;
    case EnflamePixFormat::PIX_BGR24:
      return AV_PIX_FMT_BGR24;  // RGB24
    default:;
  }
  return AV_PIX_FMT_NONE;
}

EnflamePixFormat pixTras2(AVPixelFormat pix) {
  switch (pix) {
    case AVPixelFormat::AV_PIX_FMT_YUV420P || AVPixelFormat::AV_PIX_FMT_YUVJ420P:
      return EnflamePixFormat::PIX_YUV420;
    case AVPixelFormat::AV_PIX_FMT_BGR24:
      return EnflamePixFormat::PIX_BGR24;
    default:;
  }
  return EnflamePixFormat::PIX_NONE;
}
}  // namespace enflame