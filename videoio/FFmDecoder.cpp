//
// Created by caizc on 6/9/22.
//

#include "../xlog/xlog.h"
#include "FFmDecoder.h"
#include "Utils.h"

namespace enflame {
FFmDecoder::FFmDecoder(const std::string &name) : Codec(name) { m_errBuf = new char[ERR_BUF_LEN]; }

FFmDecoder::~FFmDecoder() {
  if (m_errBuf) {
    delete[] m_errBuf;
    m_errBuf = nullptr;
  }
}

bool FFmDecoder::init() {
  assert(m_codecParPtr);
  // just for short var len
  auto name = m_codecParPtr->m_codecName;
  auto w = m_codecParPtr->m_width;
  auto h = m_codecParPtr->m_height;

  if (!name.empty()) {
    m_dec = avcodec_find_decoder_by_name(name.c_str());
    if (!m_dec) {
      LOG_ERROR("avcodec not find_decoder_by_name:[{}]", name);
      return false;
    }
  }

  LOG_INFO("Find codec :{}", name);
  m_decCtx = avcodec_alloc_context3(m_dec);
  if (!m_decCtx) {
    LOG_INFO("avcodec_alloc_context3");
    return false;
  }

  LOG_INFO("Setting decoder Paramers");
  // sometimes detete it
  if (m_codecParPtr->m_codecName == "h264dec_omx" || m_codecParPtr->m_codecName == "h264") {
    if (m_codecParPtr->codecParm) {
      m_params = (AVCodecParameters *)m_codecParPtr->codecParm;
    }

    AVCodecParameters par;
    if (m_params) {
      par = *m_params;
    } else {
      memset(&par, 0, sizeof(par));
      par.width = m_codecParPtr->m_width;
      par.height = m_codecParPtr->m_height;
      par.codec_type = AVMEDIA_TYPE_VIDEO;
    }
    int ret = avcodec_parameters_to_context(m_decCtx, &par);
    if (ret < 0) {
      printErrMsg(ret, "avcodec_parameters_to_context");
      return false;
    }

    // this three will cause decoder's pts became very big
    // so never open them
    // m_decCtx->time_base = {1, AV_TIME_BASE};
    // m_decCtx->pkt_timebase = {1, AV_TIME_BASE};
    // m_decCtx->framerate = {1, 25};

    m_decCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    //  m_decCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  }
  return true;
}

bool FFmDecoder::open() {
  assert(m_decCtx);
  int ret = -1;
  ret = avcodec_open2(m_decCtx, m_dec, nullptr);
  if (ret < 0) {
    printErrMsg(ret, "avcodec_open2");
    return false;
  }

  m_frame = av_frame_alloc();
  m_pkt = av_packet_alloc();
  m_stateFlag = StateType ::OPENED;
  m_codecParPtr->codecParm = (void *)m_decCtx;
  LOG_INFO("Decoder : {} open success.", m_codecParPtr->m_codecName);
  return true;
}

bool FFmDecoder::destory() {
  if (m_decCtx) {
    avcodec_close(m_decCtx);
    avcodec_free_context(&m_decCtx);
    m_decCtx = nullptr;
  }

  if (m_frame) av_frame_free(&m_frame);
  if (m_pkt) av_packet_free(&m_pkt);
  m_stateFlag = StateType ::CLOSED;
  LOG_INFO("Destory decoder.");
  return true;
}

bool FFmDecoder::decode(VideoPacketPtr in, VideoFramePtrArr &out) {
  assert(m_stateFlag == StateType::OPENED);

  int ret = -1;

  //  if (m_params && in->keyFrame) {
  //    m_pkt->data = (uint8_t *)av_malloc(m_params->extradata_size);
  //    memcpy(m_pkt->data, m_params->extradata, m_params->extradata_size);
  //    m_pkt->size = m_params->extradata_size;
  //    printHex(m_pkt->data, 15);
  //    // send pkt to decode
  //    if (m_pkt->data) {
  //      // LOG_INFO("send pps");
  //      auto ret = avcodec_send_packet(m_decCtx, m_pkt);
  //      if (ret < 0) {
  //        printErrMsg(ret, "avcodec_send_packet");
  //        return false;
  //      }
  //    }
  //    av_packet_unref(m_pkt);
  //  }

  m_pkt->size = in->size;
  m_pkt->data = in->data;
  m_pkt->pts = in->pts;
  m_pkt->dts = in->dts;
  m_pkt->time_base = {1, in->time_base};
  m_pkt->duration = in->duration;
  if (in->keyFrame) m_pkt->flags |= AV_PKT_FLAG_KEY;
  in->data = nullptr;
  in->size = 0;

  // printHex(m_pkt->data, 10);

  // send pkt to decode
  if (m_pkt->data) {
    // LOG_INFO("send data,pts:{},dts:{}", m_pkt->pts, m_pkt->dts);
    auto ret = avcodec_send_packet(m_decCtx, m_pkt);
    if (ret < 0) {
      printErrMsg(ret, "avcodec_send_packet");
      return false;
    }

    // if (m_pkt->data) {
    //   free(m_pkt->data);
    //   m_pkt->data = nullptr;
    //   m_pkt->size = 0;
    //   m_pkt->pts = m_pkt->dts = 0;
    // }
    m_sendFrameIdx++;
    av_packet_unref(m_pkt);
  }

  ret = 0;
  while (ret >= 0) {
    // LOG_INFO("receive data-----------");
    av_frame_unref(m_frame);
    ret = avcodec_receive_frame(m_decCtx, m_frame);
    // no more output frame
    if (ret < 0) {
      if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        // LOG_INFO("receive data->>>>>>>>");
        break;
      } else {
        printErrMsg(ret, "avcodec_receive_frame");
        return false;
      }
    }

    auto ele = std::make_shared<VideoFrame>();
    ele->width = m_frame->width;
    ele->height = m_frame->height;
    ele->pts = m_frame->pts;
    //    ele->pts = m_pts;
    //    m_pts += 40;
    //    ele->dur = 40;                  // m_frame->pkt_duration;
    ele->time_base = EN_TIME_BASE;  // m_frame->time_base.den;
    ele->format = pixTras2((AVPixelFormat(m_frame->format)));

    // LOG_INFO("receive data,dts:{} pts:{},dur:{}", m_frame->pts, m_frame->pts,
    // m_frame->pkt_duration);

    for (int i = 0; i < MAX_NUM; ++i) {
      ele->linesize[i] = m_frame->linesize[i];
      auto size = size_t(ele->linesize[i] * ele->height);
      if (size > 0) {
        ele->data[i] = (uint8_t *)malloc(size);
        memset(ele->data[i], 0, size);
        memcpy(ele->data[i], m_frame->data[i], size);
      }
    }

    // m_inner.push_back(ele);
    out.push_back(ele);
    m_frameIdx++;
  }  // while

  //  std::sort(m_inner.begin(), m_inner.end(),
  //            [](const VideoFramePtr l, const VideoFramePtr r) { return (l->pts < r->pts); });
  //
  //  while (m_inner.size() >= 10) {
  //    out.push_back(m_inner.front());
  //    m_inner.erase(m_inner.begin());
  //  }

  return (!out.empty());
}

void FFmDecoder::printErrMsg(int ret, const std::string &funName) {
  if (ret >= 0) return;
  av_strerror(ret, m_errBuf, ERR_BUF_LEN);
  LOG_ERROR("{} < 0 of {} for {}...", funName, ret, m_errBuf);
}

}  // namespace enflame