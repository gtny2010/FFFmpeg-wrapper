#include "xmylog.h"
namespace xlog {
xlogger::xlogger(std::string name) : spdlog::logger(std::move(name)) {}
void xlogger::sink_it_(const spdlog::details::log_msg &msg) {
  spdlog::logger::sink_it_(msg);
}
void xlogger::flush_() { spdlog::logger::flush_(); }
} // namespace xlog
