#if OMPT_USE_NUMA_DEVICE_AFFINITY

#ifndef _OMPTARGET_NUMA_H_
#define _OMPTARGET_NUMA_H_

#include <stdint.h>
#include <numa.h>

struct __tgt_numa_info {
    int32_t Available = 0;
    int32_t ConfiguredNodes;
    int32_t **NumaDistanceTable;
};

#endif // _OMPTARGET_NUMA_H_

#endif // OMPT_USE_NUMA_DEVICE_AFFINITY
