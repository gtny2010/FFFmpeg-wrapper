//
// Created by caizc on 6/8/22.
//

#include "FFmMux.h"

#include <assert.h>

#include "../xlog/xlog.h"
#include "Utils.h"

namespace enflame {

static AVCodecID ffmcodecID(EnflameCodecID id) {
  if (id == EnflameCodecID::CODEC_ID_H264)
    return AV_CODEC_ID_H264;
  else if (id == EnflameCodecID::CODEC_ID_MPEG4)
    return AV_CODEC_ID_MPEG4;
  else
    return AV_CODEC_ID_NONE;
}

FFmMux::FFmMux(const std::string &name) : Mux(name) { m_errBuf = new char[ERR_BUF_LEN]; }

FFmMux::~FFmMux() {
  if (m_errBuf) {
    delete[] m_errBuf;
    m_errBuf = nullptr;
  }
}

bool FFmMux::initMux(VideoCfgPtr cfg) {
  assert(cfg);
  m_url = cfg->url;
  m_width = cfg->width;
  m_height = cfg->height;
  m_cId = cfg->cId;
  return true;
}

bool FFmMux::openMux() {
  LOG_INFO("Open output url:{}", m_url);
  int ret = avformat_alloc_output_context2(&m_avfmtCtx, nullptr, "flv", m_url.c_str());
  if (ret < 0) {
    LOG_ERROR("Could not deduce output format from file:{}", m_url);
    printErrMsg(ret, "avformat_alloc_output_context2");
    return false;
  }

  m_outStream = avformat_new_stream(m_avfmtCtx, nullptr);
  if (!m_outStream) {
    LOG_ERROR("Failed allocating output stream");
    avformat_free_context(m_avfmtCtx);
    return false;
  }

  if (m_codeCtx) {
    LOG_INFO("Copy codecpar form codec Ctx.");
    int ret = avcodec_parameters_from_context(m_outStream->codecpar, m_codeCtx);
    if (ret < 0) {
      LOG_ERROR("Failed to copy context from input to output stream codec context");
      printErrMsg(ret, "avcodec_parameters_from_context");
      avformat_free_context(m_avfmtCtx);
      return false;
    }
  } else if (m_inStream) {
    LOG_INFO("Copy codecpar form instream.");
    ret = avcodec_parameters_copy(m_outStream->codecpar, m_inStream->codecpar);
    if (ret < 0) {
      LOG_ERROR("Failed copy stream codecpar");
      printErrMsg(ret, "avcodec_parameters_copy");
      avformat_free_context(m_avfmtCtx);
      return false;
    }

  } else {
    LOG_INFO("set codecpar by user.");
    m_outStream->codecpar->width = m_width;
    m_outStream->codecpar->height = m_height;
    m_outStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    m_outStream->codecpar->codec_id = ffmcodecID(m_cId);
    m_outStream->codecpar->format = AV_PIX_FMT_YUV420P;
    m_outStream->codecpar->codec_tag = 0;
    m_outStream->codecpar->extradata = nullptr;
    m_outStream->codecpar->extradata_size = 0;
    m_outStream->time_base = {1, 1000};

    m_outStream->id = m_avfmtCtx->nb_streams - 1;
    m_outStream->time_base = {1, 1000};
  }
  m_outStream->codecpar->codec_tag = 0;

  if (!(m_avfmtCtx->oformat->flags & AVFMT_NOFILE)) {
    ret = avio_open(&m_avfmtCtx->pb, m_url.c_str(), AVIO_FLAG_WRITE);
    if (ret < 0) {
      LOG_ERROR("Could not open output URL '%s'", m_url);
      printErrMsg(ret, "avio_open");
      goto end;
    }
  }

  ret = avformat_write_header(m_avfmtCtx, nullptr);
  if (ret < 0) {
    printErrMsg(ret, "avformat_write_header");
    goto end;
  }
  m_stateFlag = StateType ::OPENED;

  m_pkt = av_packet_alloc();
  av_dump_format(m_avfmtCtx, 0, m_url.c_str(), 1);
  LOG_INFO("Open output success.{}", m_url);
  return true;

end:
  if (m_avfmtCtx) avformat_free_context(m_avfmtCtx);
  return false;
}

bool FFmMux::destory() {
  m_stateFlag = StateType ::CLOSED;
  if (m_avfmtCtx) {
    av_write_trailer(m_avfmtCtx);
    avformat_close_input(&m_avfmtCtx);
    if (m_avfmtCtx && !(m_avfmtCtx->flags & AVFMT_NOFILE)) avio_close(m_avfmtCtx->pb);
  }

  if (m_pkt) {
    av_packet_unref(m_pkt);
    av_packet_free(&m_pkt);
  }

  LOG_INFO("Destory Mux");
  return true;
}

bool FFmMux::mux(VideoPacketPtr in) {
  assert(m_stateFlag == StateType::OPENED);
  if (!in) return false;

  av_packet_unref(m_pkt);
  int ret = -1;
  m_pkt->size = in->size;
  m_pkt->data = in->data;

  m_pkt->pts = av_rescale_q_rnd(in->pts, EN_TIME_BASE_Q, m_outStream->time_base,
                                (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
  m_pkt->dts = av_rescale_q_rnd(in->dts, EN_TIME_BASE_Q, m_outStream->time_base,
                                (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
  m_pkt->duration = av_rescale_q(in->duration, EN_TIME_BASE_Q, m_outStream->time_base);

  if (m_pkt->pts < 0) m_pkt->pts = 0;
  if (m_pkt->dts < 0) m_pkt->dts = 0;
  if (m_pkt->duration <= 0) m_pkt->duration = 40;

  //  m_pkt->pts = m_pkt->dts = m_pts;
  //  m_pts += 40;

  if (in->keyFrame) {
    m_pkt->flags |= AV_PKT_FLAG_KEY;
  }
  m_pkt->pos = -1;
  m_pkt->time_base = m_outStream->time_base;

  // LOG_INFO("pts:{},dts:{},dur:{}", m_pkt->dts, m_pkt->pts, m_pkt->duration);
  // printHex(m_pkt->data, 15);

  ret = av_interleaved_write_frame(m_avfmtCtx, m_pkt);
  if (ret < 0) {
    LOG_ERROR("Failed to mux.");
    printErrMsg(ret, "av_interleaved_write_frame");
    return false;
  }

  return true;
}

void FFmMux::printErrMsg(int ret, const std::string &funName) {
  if (ret >= 0) return;
  av_strerror(ret, m_errBuf, ERR_BUF_LEN);
  LOG_ERROR("{} < 0 of {} for {}...", funName, ret, m_errBuf);
}

}  // namespace enflame