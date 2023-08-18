//
// Created by caizc on 6/7/22.
//

#ifndef TESTDAMEON_UTILS_H
#define TESTDAMEON_UTILS_H
#include "Types.h"

namespace enflame {
void clearupFrames(VideoFramePtrArr& arr);
void clearupPkts(VideoPacketPtrArr& arr);
void printHex(uint8_t* data, int len);

}  // namespace enflame
#endif  // TESTDAMEON_UTILS_H
