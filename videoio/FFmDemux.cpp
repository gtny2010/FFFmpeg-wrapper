//
// Created by caizc on 6/7/22.
//

#include <assert.h>

#include "../xlog/xlog.h"
#include "FFmDemux.h"
#include "Utils.h"
namespace enflame {

FFmDemux::FFmDemux(const std::string &name) : Demux(name) { m_errBuf = new char[ERR_BUF_LEN]; }

FFmDemux::~FFmDemux() {
  if (m_errBuf) {
    delete[] m_errBuf;
    m_errBuf = nullptr;
  }
}

bool FFmDemux::initDemux(VideoCfgPtr cfg) {
  m_url = cfg->url;
  return true;
}

bool FFmDemux::openDemux() {
  assert(m_url.size() > 0);
  int ret = -1;

  m_avformatCtx = avformat_alloc_context();
  if (!m_avformatCtx) return false;

  AVDictionary *dic = nullptr;
  if (std::string::npos != m_url.find("rtsp")) {
    av_dict_set(&dic, "buffer_size", "4096", 0);
    av_dict_set(&dic, "rtsp_transport", "tcp", 0);
    av_dict_set(&dic, "muxdelay", "10", 0);  // seconds
    av_dict_set(&dic, "stimeout", "20000000", 0);
    LOG_INFO("ffdict set,buffer_size:4096,rtsp_transport:tcp");
    LOG_INFO("ffdict set,muxdelay:10,stimeout:20000000");
  }

  ret = avformat_open_input(&m_avformatCtx, m_url.c_str(), NULL, &dic);
  if (ret != 0) {
    printErrMsg(ret, "avformat_open_input");
    return false;
  }
  av_dict_free(&dic);
  ret = avformat_find_stream_info(m_avformatCtx, nullptr);
  if (ret != 0) {
    printErrMsg(ret, "avformat_open_input");
    destory();
    return false;
  }

  for (auto i = 0; i < m_avformatCtx->nb_streams; ++i) {
    AVStream *stream = m_avformatCtx->streams[i];
    if (!stream) continue;
    if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      m_videoIndex = i;
      break;
    }
  }

  if (m_videoIndex < 0) {
    LOG_ERROR("m_videoIndex < 0");
    destory();
    return false;
  }

  m_videoStream = m_avformatCtx->streams[m_videoIndex];
  AVCodecParameters *par = m_avformatCtx->streams[m_videoIndex]->codecpar;
  if (par->width <= 0 || par->height <= 0) {
    LOG_ERROR("width or height < 0 ,width:{},height:{}", par->width, par->height);
    destory();
    return false;
  }
  m_width = par->width;
  m_height = par->height;
  m_codecId = codeIDTras(par->codec_id);

  av_dump_format(m_avformatCtx, m_videoIndex, m_url.c_str(), 0);
  m_pkt = av_packet_alloc();
  m_startTime = av_gettime();
  m_stateFlag = StateType ::OPENED;
  return true;
}

bool FFmDemux::destory() {
  m_stateFlag = StateType ::CLOSED;
  if (m_avformatCtx) {
    avformat_close_input(&m_avformatCtx);
    avformat_free_context(m_avformatCtx);
  }
  if (m_pkt) {
    av_packet_unref(m_pkt);
    av_packet_free(&m_pkt);
  }
  LOG_INFO("Destory Demux.");
  return true;
}

bool FFmDemux::demux(VideoPacketPtr out) {
  assert(m_avformatCtx);
  int ret = -1;
  while (true) {
    av_packet_unref(m_pkt);
    ret = av_read_frame(m_avformatCtx, m_pkt);
    if (ret < 0) {
      if (ret == AVERROR_EOF) {
        m_eof = true;
        av_packet_unref(m_pkt);
        LOG_INFO("ffread for end of file,set eof.");
        return false;
      }
      // reopen
      printErrMsg(ret, "av_read_frame");
      return false;
    }

    // printHex(m_videoStream->codecpar->extradata, 15);
    // LOG_INFO(" cap11 pts:{},dts:{}", m_pkt->pts, m_pkt->dts);
    if (m_pkt->stream_index != m_videoIndex) continue;

    // this for file delay
    AVRational time_base = m_avformatCtx->streams[m_videoIndex]->time_base;
    int64_t ptsTime = av_rescale_q(m_pkt->dts, time_base, AV_TIME_BASE_Q);
    int64_t nowTime = av_gettime() - m_startTime;
    if (!m_firstPktFlag) {
      m_firstPtsTime = ptsTime;
      m_firstPktFlag = true;
    }

    if (ptsTime - m_firstPtsTime > nowTime) {
      // LOG_INFO("pkt begin sleep,pts:{},nowTime:{}", ptsTime, nowTime);
      av_usleep(static_cast<unsigned>(ptsTime - m_firstPtsTime - nowTime));
    }

    // output need input's pts_timebase={1,1000000}
    m_pkt->pts = av_rescale_q(m_pkt->pts, time_base, EN_TIME_BASE_Q);
    m_pkt->dts = av_rescale_q(m_pkt->dts, time_base, EN_TIME_BASE_Q);
    m_pkt->duration = av_rescale_q(m_pkt->duration, time_base, EN_TIME_BASE_Q);

    out->size = m_pkt->size;
    out->data = (uint8_t *)av_malloc(out->size);
    memcpy(out->data, m_pkt->data, out->size);
    out->pts = m_pkt->pts;
    out->dts = m_pkt->dts;
    out->duration = m_pkt->duration;
    out->time_base = EN_TIME_BASE;
    out->keyFrame = m_pkt->flags & AV_PKT_FLAG_KEY;
    // LOG_INFO(" cap pts:{},dts:{}", out->pts, out->dts);
    m_frameIdx++;
    return true;
  }  // while
}

EnflameCodecID FFmDemux::codeIDTras(AVCodecID ID) {
  switch (ID) {
    case AVCodecID ::AV_CODEC_ID_H264:
      return EnflameCodecID::CODEC_ID_H264;
    case AVCodecID ::AV_CODEC_ID_MPEG4:
      return EnflameCodecID::CODEC_ID_MPEG4;
  }
  return EnflameCodecID ::CODEC_ID_NONE;
}

void FFmDemux::printErrMsg(int ret, const std::string &funName) {
  if (ret >= 0) return;
  av_strerror(ret, m_errBuf, ERR_BUF_LEN);
  LOG_ERROR("{} < 0 of {} for {}...", funName, ret, m_errBuf);
}

}  // namespace enflame