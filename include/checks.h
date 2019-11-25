#ifndef _NVLM_CHECKS_H_
#define _NVLM_CHECKS_H_

#include "nvml.h"

#define NV_CHECK(call) do {                               \
    nvmlReturn_t ret = call;                              \
    if (ret != NVML_SUCCESS){                             \
        printf("[%s : %d] NV_CHECK error, return: %s\n",  \
               __FILE__, __LINE__, nvmlErrorString(ret)); \
        exit(EXIT_FAILURE);                               \
    }                                                     \
} while (0);  

#define NVLM_CHECK(call) do {                             \
    nvlmResult_t res = call;                              \
    if (res != NVLM_SUCCESS){                             \
        printf("[%s:%d] NVLM_CHECK error.\n",             \
               __FILE__, __LINE__);                       \
        exit(EXIT_FAILURE);                               \
    }                                                     \
} while (0);                                              \

#endif
