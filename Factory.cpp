//
// Created by caizc on 6/15/22.
//

#include "Factory.h"

#include "videoio/FFmDemux.h"
#include "videoio/FFmMux.h"
#include "videoio/H264Mux.h"
#include "videoio/YuvDemux.h"
#include "videoio/YuvMux.h"

namespace enflame {
DemuxerWorkerPtr createFFmDemuxerWrk() {
  auto ffmDemuxer = FFmDemux::create("ffm-demuxer");
  return DemuxerWorker::create(ffmDemuxer);
}

MuxWorkerPtr createFFmMuxerWrk() {
  auto ffmMuxer = FFmMux::create("ffm-muxer");
  return MuxWorker::create(ffmMuxer);
}

MuxWorkerPtr createYuvMuxerWrk() {
  auto ffmMuxer = YuvMux::create("yuv-muxer");
  return MuxWorker::create(ffmMuxer);
}
MuxWorkerPtr createH264MuxerWrk() {
  auto ffmMuxer = H264Mux::create("h264-muxer");
  return MuxWorker::create(ffmMuxer);
}

DemuxerWorkerPtr createYuvDemuxerWrk() {
  auto yuvDemuxer = YuvDemux::create("yuv-demuxer");
  return DemuxerWorker::create(yuvDemuxer);
}
}  // namespace enflame