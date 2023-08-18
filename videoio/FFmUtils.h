//
// Created by caizc on 6/9/22.
//

#ifndef TESTDAMEON_FFMUTILS_H
#define TESTDAMEON_FFMUTILS_H

#ifdef __cplusplus
extern "C" {
#endif
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>

#include "libavcodec/avcodec.h"
#include "libavcodec/bsf.h"
#include "libavformat/avformat.h"
#include "libavutil/buffer.h"
#include "libavutil/error.h"
#include "libavutil/imgutils.h"
#include "libavutil/mem.h"
#include "libavutil/opt.h"
#include "libavutil/time.h"
#include "libswscale/swscale.h"
#ifdef __cplusplus
};
#endif

#include <string>

#include "Types.h"

namespace enflame {
AVPixelFormat pixTras(EnflamePixFormat color);
EnflamePixFormat pixTras2(AVPixelFormat color);
}  // namespace enflame

#endif  // TESTDAMEON_FFMUTILS_H
