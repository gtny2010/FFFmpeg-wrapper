#include <assert.h>

#include "../xlog/xlog.h"
#include "FFmUtils.h"
#include "SwscaleFilter.h"
namespace enflame {
SwscaleFilter::SwscaleFilter(const std::string &name) : Filter(name) {
  m_errBuf = new char[ERR_BUF_LEN];
  m_RGB = av_frame_alloc();
  m_frame = av_frame_alloc();
}

SwscaleFilter::~SwscaleFilter() {
  if (m_errBuf) {
    delete[] m_errBuf;
    m_errBuf = nullptr;
  }
  if (m_RGB) {
    av_frame_free(&m_RGB);
    m_RGB = nullptr;
  }
}

bool SwscaleFilter::init() {
  m_numBytes =
      av_image_get_buffer_size(pixTras(m_swsCfg->repix), m_swsCfg->width, m_swsCfg->height, 1);
  m_buffer = (uint8_t *)av_malloc(m_numBytes);
  m_imgConvertCtx = sws_alloc_context();
  if (sws_init_context(m_imgConvertCtx, nullptr, nullptr) < 0) {
    return false;
  }
  memset(m_buffer, 0, m_numBytes);
  return true;
}
bool SwscaleFilter::open() {
  int ret = -1;
  memset(m_RGB, 0, sizeof(m_RGB));
  ret = av_image_fill_arrays(m_RGB->data, m_RGB->linesize, m_buffer, pixTras(m_swsCfg->repix),
                             m_swsCfg->rewidth, m_swsCfg->reheight, 1);
  if (ret < 0) {
    return false;
  }
  auto method = SWS_BILINEAR;
  if (m_swsCfg->pix == EnflamePixFormat::PIX_YUV420) {
    method = SWS_BICUBIC;
  }
  m_imgConvertCtx =
      sws_getContext(m_swsCfg->width, m_swsCfg->height, pixTras(m_swsCfg->pix), m_swsCfg->rewidth,
                     m_swsCfg->reheight, pixTras(m_swsCfg->repix), method, NULL, NULL, NULL);
  m_stateFlag = StateType::OPENED;
  return true;
}

bool SwscaleFilter::destory() {
  av_free(m_buffer);
  av_frame_free(&m_RGB);
  if (m_imgConvertCtx) {
    sws_freeContext(m_imgConvertCtx);
  }
  m_stateFlag = StateType::CLOSED;
  return true;
}

bool SwscaleFilter::filter(VideoFramePtr in, VideoFramePtr &out) {
  if (!in) return false;
  assert(m_stateFlag == StateType::OPENED);
  av_frame_unref(m_frame);
  // av_frame_unref(m_RGB);
  int ret = -1;
  m_frame->time_base = {1, in->time_base};
  m_frame->pts = in->pts;
  m_frame->width = in->width;
  m_frame->height = in->height;
  m_frame->format = pixTras(in->format);

  for (int i = 0; i < MAX_NUM; ++i) {
    m_frame->linesize[i] = in->linesize[i];
    m_frame->data[i] = in->data[i];
    in->data[i] = nullptr;
    in->linesize[i] = 0;
  }

  ret = sws_scale(m_imgConvertCtx, (const uint8_t *const *)m_frame->data, m_frame->linesize, 0,
                  m_frame->height, m_RGB->data, m_RGB->linesize);
  if (ret < 0) {
    printErrMsg(ret, "sws_scale");
    LOG_INFO("sws_scale err!");
  }

  auto ele = std::make_shared<VideoFrame>();
  ele->width = m_swsCfg->rewidth;
  ele->height = m_swsCfg->reheight;
  ele->pts = in->pts;
  //    ele->pts = m_pts;
  //    m_pts += 40;
  //    ele->dur = 40;
  ele->time_base = EN_TIME_BASE;  // m_frame->time_base.den;
  ele->format = m_swsCfg->repix;

  // LOG_INFO("receive data,dts:{} pts:{},dur:{}", m_frame->pts, m_frame->pts,
  // m_frame->pkt_duration);

  for (int i = 0; i < MAX_NUM; ++i) {
    ele->linesize[i] = m_RGB->linesize[i];
    auto size = size_t(ele->linesize[i] * ele->height);
    if (size > 0) {
      ele->data[i] = (uint8_t *)malloc(size);
      memset(ele->data[i], 0, size);
      memcpy(ele->data[i], m_RGB->data[i], size);
    }
  }
  out = ele;
  return (out != nullptr);
}

void SwscaleFilter::printErrMsg(int ret, const std::string &funName) {
  if (ret >= 0) return;
  av_strerror(ret, m_errBuf, ERR_BUF_LEN);
  LOG_ERROR("{} < 0 of {} for {}...", funName, ret, m_errBuf);
}

}  // namespace enflame
