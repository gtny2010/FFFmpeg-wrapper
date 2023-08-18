//
// Created by caizc on 6/17/22.
//

#include "BsfFIlter.h"

#include "../xlog/xlog.h"
#include "Utils.h"

namespace enflame {

BsfFilter::BsfFilter(const std::string &name) : Filter(name) { m_errBuf = new char[ERR_BUF_LEN]; }
BsfFilter::~BsfFilter() {
  if (m_errBuf) {
    delete[] m_errBuf;
    m_errBuf = nullptr;
  }
}

bool BsfFilter::init() {
  const AVBitStreamFilter *bsf;

  if (m_codecParam->codec_id == AV_CODEC_ID_H264) {
    bsf = av_bsf_get_by_name("h264_mp4toannexb");
    if (!bsf) {
      LOG_ERROR("h264_mp4toannexb filter get failed");
      return false;
    }
  } else {
    bsf = av_bsf_get_by_name("hevc_mp4toannexb");
    if (!bsf) {
      LOG_INFO("hevc_mp4toannexb filter get failed");
      return false;
    }
  }

  if (av_bsf_alloc(bsf, &m_bsfCtx) != 0) {
    return false;
  }

  m_bsfCtx->par_in->codec_id = m_codecParam->codec_id;
  m_bsfCtx->par_in->codec_type = m_codecParam->codec_type;
  m_bsfCtx->par_in->codec_tag = m_codecParam->codec_tag;
  m_bsfCtx->par_in->width = m_codecParam->width;
  m_bsfCtx->par_in->height = m_codecParam->height;
  m_bsfCtx->par_in->format = m_codecParam->format;

  m_bsfCtx->par_in->field_order = m_codecParam->field_order;
  m_bsfCtx->par_in->color_range = m_codecParam->color_range;
  m_bsfCtx->par_in->color_primaries = m_codecParam->color_primaries;
  m_bsfCtx->par_in->color_trc = m_codecParam->color_trc;
  m_bsfCtx->par_in->color_space = m_codecParam->color_space;
  m_bsfCtx->par_in->chroma_location = m_codecParam->chroma_location;
  m_bsfCtx->par_in->sample_aspect_ratio = m_codecParam->sample_aspect_ratio;
  m_bsfCtx->par_in->video_delay = m_codecParam->video_delay;

  //  if (m_codecParam->extradata) {
  //    m_bsfCtx->par_in->extradata = (uint8_t *)av_mallocz(m_codecParam->extradata_size +
  //    AV_INPUT_BUFFER_PADDING_SIZE); if (!m_bsfCtx->par_in->extradata) return AVERROR(ENOMEM);
  //    memcpy(m_bsfCtx->par_in->extradata, m_codecParam->extradata, m_codecParam->extradata_size);
  //    m_bsfCtx->par_in->extradata_size = m_codecParam->extradata_size;
  //  }

  // avcodec_parameters_from_context(m_bsfCtx->par_in, m_codecCtx);

  return true;
}

bool BsfFilter::open() {
  if (av_bsf_init(m_bsfCtx) != 0) {
    LOG_ERROR("av_bsf_init fail.");
    return false;
  }
  return true;
}

bool BsfFilter::destory() {
  if (m_bsfCtx) {
    av_bsf_free(&m_bsfCtx);
  }
  return true;
}
bool BsfFilter::filter(VideoPacketPtr in, VideoPacketPtr &out) {
  if (!in) return false;

  AVPacket pkt1, pkt2;

  pkt1.size = in->size;
  pkt1.data = in->data;
  pkt1.pts = in->pts;
  pkt1.dts = in->dts;
  pkt1.time_base = {1, in->time_base};
  pkt1.duration = in->duration;
  if (in->keyFrame) pkt1.flags |= AV_PKT_FLAG_KEY;
  in->data = nullptr;
  in->size = 0;
  // printHex(pkt1.data, 15);
  auto ret = av_bsf_send_packet(m_bsfCtx, &pkt1);
  if (ret < 0) {
    printErrMsg(ret, "av_bsf_send_packet");
    return false;
  }

  if (pkt1.data) {
    free(pkt1.data);
    pkt1.data = nullptr;
    pkt1.size = 0;
    pkt1.pts = pkt1.dts = 0;
  }

  AVPacket pktFiltered;
  auto nbsfRet = av_bsf_receive_packet(m_bsfCtx, &pkt2);
  if (nbsfRet < 0) {
    printErrMsg(ret, "av_bsf_receive_packet");
    return false;
  }
  // printHex(pkt2.data, 15);
  out->size = pkt2.size;
  out->data = (uint8_t *)av_malloc(out->size);
  memcpy(out->data, pkt2.data, out->size);
  out->pts = pkt2.pts;
  out->dts = pkt2.dts;
  out->duration = pkt2.duration;
  out->time_base = EN_TIME_BASE;
  out->keyFrame = pkt2.flags & AV_PKT_FLAG_KEY;

  return true;
}
bool BsfFilter::filter(VideoFramePtr in, VideoFramePtr &out) { return true; }

void BsfFilter::printErrMsg(int ret, const std::string &funName) {
  if (ret >= 0) return;
  av_strerror(ret, m_errBuf, ERR_BUF_LEN);
  LOG_ERROR("{} < 0 of {} for {}...", funName, ret, m_errBuf);
}
}  // namespace enflame