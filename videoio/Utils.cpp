//
// Created by caizc on 6/10/22.
//
#include "Utils.h"
namespace enflame {
void clearupFrames(VideoFramePtrArr &arr) {
  for (int i = 0; i < arr.size(); ++i) {
    if (arr[i]) {
      for (int j = 0; j < MAX_NUM; ++j) {
        void *p = arr[i]->data[j];
        free(p);
        arr[i]->data[j] = nullptr;
        arr[i]->linesize[j] = 0;
      }
    }
  }
}

void clearupPkts(VideoPacketPtrArr &arr) {
  for (int i = 0; i < arr.size(); ++i) {
    if (arr[i]) {
      free(arr[i]->data);
      arr[i]->data = nullptr;
      arr[i]->size = 0;
    }
  }
}

void printHex(uint8_t *data, int len) {
  if (!data) return;
  for (int i = 0; i < len; ++i) {
    printf("%x ", data[i]);
  }
  printf("\n");
}

}  // namespace enflame