//
// Created by caizc on 6/15/22.
//

#ifndef TESTDAMEON_FACTORY_H
#define TESTDAMEON_FACTORY_H

#include "DemuxerWorker.h"
#include "MuxWorker.h"
namespace enflame {
DemuxerWorkerPtr createFFmDemuxerWrk();
DemuxerWorkerPtr createYuvDemuxerWrk();
MuxWorkerPtr createFFmMuxerWrk();
MuxWorkerPtr createYuvMuxerWrk();
MuxWorkerPtr createH264MuxerWrk();
}  // namespace enflame

#endif  // TESTDAMEON_FACTORY_H
