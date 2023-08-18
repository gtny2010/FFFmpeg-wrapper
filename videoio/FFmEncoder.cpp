//
// Created by caizc on 6/9/22.
//

#include "../xlog/xlog.h"
#include "FFmEncoder.h"

namespace enflame {
FFmEncoder::FFmEncoder(const std::string &name) : Codec(name) { m_errBuf = new char[ERR_BUF_LEN]; }
FFmEncoder::~FFmEncoder() {
  if (m_errBuf) {
    delete[] m_errBuf;
    m_errBuf = nullptr;
  }
}

bool FFmEncoder::init() {
  assert(m_codecParPtr);
  // just for short var len
  auto name = m_codecParPtr->m_codecName;
  auto w = m_codecParPtr->m_width;
  auto h = m_codecParPtr->m_height;

  if (!name.empty()) {
    m_enc = avcodec_find_encoder_by_name(name.c_str());
    if (!m_enc) {
      LOG_INFO("avcodec_find_decoder_by_name");
      return false;
    }
  }

  LOG_INFO("Find codec :{}", name);
  m_encCtx = avcodec_alloc_context3(m_enc);
  if (!m_encCtx) {
    LOG_INFO("avcodec_alloc_context3");
    return false;
  }

  LOG_INFO("Setting encoder Paramers");
  // allocate codec context
  m_encCtx->codec_id = m_enc->id;
  m_encCtx->codec_type = AVMEDIA_TYPE_VIDEO;
  m_encCtx->pix_fmt = pixTras(m_codecParPtr->format);
  //  m_encCtx->bit_rate = 400000;
  //  m_encCtx->rc_buffer_size = m_codecParPtr->m_width * m_codecParPtr->m_height * 3;

  m_encCtx->width = m_codecParPtr->m_width;
  m_encCtx->height = m_codecParPtr->m_height;
  //  m_encCtx->qmin = 32;
  //  m_encCtx->qmax = 36;
  m_encCtx->gop_size = 15;
  m_encCtx->has_b_frames = 0;

  m_encCtx->time_base.num = 1;
  m_encCtx->time_base.den = 30;  // AV_TIME_BASE;

  m_encCtx->framerate.num = 30;
  m_encCtx->framerate.den = 1;

  // m_encCtx->sample_aspect_ratio.den = 1;
  // m_encCtx->sample_aspect_ratio.num = 1;

  m_encCtx->profile = 100;  // default
  m_encCtx->level = 42;     // default
  //  m_encCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

  // set h264 and hevc encode option
  if (m_enc->id == AV_CODEC_ID_H264) {
    av_opt_set(m_encCtx->priv_data, "profile", "baseline", 0);
  } else if (m_enc->id == AV_CODEC_ID_HEVC) {
    av_opt_set(m_encCtx->priv_data, "profile", "main", 0);  // main 10
  }
  return true;
}

bool FFmEncoder::open() {
  assert(m_encCtx);
  int ret = -1;
  m_encCtx->thread_count = 8;
  ret = avcodec_open2(m_encCtx, m_enc, nullptr);
  if (ret < 0) {
    printErrMsg(ret, "avcodec_open2");
    LOG_INFO("Open Encoder failed {}", m_codecParPtr->m_codecName);
    return false;
  }

  m_frame = av_frame_alloc();
  m_pkt = av_packet_alloc();
  m_stateFlag = StateType ::OPENED;
  m_codecParPtr->codecParm = (void *)m_encCtx;
  LOG_INFO("Encoder : {} open success.", m_codecParPtr->m_codecName);
  return true;
}

bool FFmEncoder::destory() {
  if (m_encCtx) {
    avcodec_close(m_encCtx);
    avcodec_free_context(&m_encCtx);
    m_encCtx = nullptr;
  }

  if (m_frame) av_frame_free(&m_frame);
  if (m_pkt) av_packet_free(&m_pkt);
  m_stateFlag = StateType ::CLOSED;
  LOG_INFO("Destory decoder.");
  return true;
}

bool FFmEncoder::encode(VideoFramePtr in, VideoPacketPtrArr &out) {
  assert(m_stateFlag == StateType::OPENED);
  int ret = -1;

  if (in) {
    m_frame->time_base = {1, in->time_base};
    m_frame->pts = in->pts;
    m_frame->width = in->width;
    m_frame->height = in->height;
    m_frame->format = AV_PIX_FMT_YUV420P;  // pixTras(in->format);

    for (int i = 0; i < MAX_NUM; ++i) {
      m_frame->linesize[i] = in->linesize[i];
      m_frame->data[i] = in->data[i];
      in->data[i] = nullptr;
      in->linesize[i] = 0;
    }

    // send frame to encode
    if (m_frame->data[0]) {
      // LOG_INFO("send data,pts:{}", m_frame->pts);
      auto ret = avcodec_send_frame(m_encCtx, m_frame);
      if (ret < 0) {
        printErrMsg(ret, "avcodec_send_frame");
        return false;
      }

      // free data
      // for (int i = 0; i < MAX_NUM; ++i) {
      //   m_frame->linesize[i] = 0;
      //   if (m_frame->data[i]) free(m_frame->data[i]);
      //   m_frame->data[i] = nullptr;
      // }
      m_sendFrameIdx++;
      av_frame_unref(m_frame);
    }
  }

  ret = 0;

  if (!in && m_frameIdx > 0 && !m_sendEnd && m_eof) {
    m_sendEnd = true;
    avcodec_send_frame(m_encCtx, nullptr);
  }

  while (ret >= 0) {
    av_packet_unref(m_pkt);
    // if decode with cuvid ,m_frame->data is on GPU mem
    ret = avcodec_receive_packet(m_encCtx, m_pkt);
    // no more output frame
    if (ret < 0) {
      if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        break;
      } else {
        printErrMsg(ret, "avcodec_receive_packet");
        return false;
      }
    }
    // LOG_INFO("receive data,pts:{},dts:{}", m_pkt->pts, m_pkt->dts);
    auto ele = std::make_shared<VideoPacket>();
    ele->dts = m_pkt->dts;
    ele->pts = m_pkt->pts;
    ele->duration = m_pkt->duration;
    ele->time_base = m_pkt->time_base.den;
    ele->size = m_pkt->size;
    ele->data = (uint8_t *)av_malloc(ele->size);
    memcpy(ele->data, m_pkt->data, ele->size);
    ele->keyFrame = m_pkt->flags & AV_PKT_FLAG_KEY;
    out.push_back(ele);
    m_frameIdx++;
  }  // while

  return (!out.empty());
}

void FFmEncoder::printErrMsg(int ret, const std::string &funName) {
  if (ret >= 0) return;
  av_strerror(ret, m_errBuf, ERR_BUF_LEN);
  LOG_ERROR("{} < 0 of {} for {}...", funName, ret, m_errBuf);
}

}  // namespace enflame
