#ifndef _NVLM_UTIL_H_
#define _NVLM_UTIL_H_

#include <chrono>
#include <iostream>
#include <fstream>
#include <getopt.h>
#include <assert.h>
#include <vector>
#include <string.h>
#include <unistd.h>
//#include "logging.h"
#include "nvml.h"
#include "checks.h"
#include "database.h"

using namespace std;
using namespace std::chrono;

#define NVLM_RESET        1
#define NVLM_NOT_RESET    0
#define MAX_DEVICE_COUNT 16

typedef enum {
  NVLM_SUCCESS = 0,
  NVLM_FAIL = 1,
  NVLM_WRONG_PARAMETER = 2
} nvlmResult_t;

struct Link {
  int rank;
  unsigned long long rx0 = 0;
  unsigned long long rx1 = 0;
  unsigned long long tx0 = 0;
  unsigned long long tx1 = 0;
};

namespace nvlm {
static nvmlDevice_t device;
static char name[NVML_DEVICE_NAME_BUFFER_SIZE];
static int topo[MAX_DEVICE_COUNT][NVML_NVLINK_MAX_LINKS];

class Operations {
  public:
    Operations();

    ~Operations() {};

    nvlmResult_t GetLinkStatus();

    // nvlmResult_t GetLinkUtilRates();

    nvlmResult_t FillTopoMatrix();

    /**
     * @brief get global nvlink topology.
     *
     * GPU[0]:  C0->G3  C1->G3  C2->G2  C3->G1  C4->G2  C5->G1
     * GPU[1]:  C0->G0  C1->G3  C2->G2  C3->G2  C4->G3  C5->G0
     * GPU[2]:  C0->G1  C1->G1  C2->G0  C3->G3  C4->G0  C5->G3
     * GPU[3]:  C0->G2  C1->G1  C2->G0  C3->G0  C4->G1  C5->G2
     *
     */
    nvlmResult_t GetNvlinkTopo();

    nvlmResult_t GetLinkBandwidth();
    
    nvlmResult_t GetLinkSetBandwidth();

    /**
     * @brief display current GPU utilization, each sample period may between 1
     * second and 1/6 second, depending on the product being quired.
     */
    nvlmResult_t GetGpuUtilRates();

    /**
     * @brief display help inforamtion.
     */
    nvlmResult_t GetHelp();

    nvlmResult_t GetRemotePciInfo(int index, unsigned int link, nvmlPciInfo_t &info);

    nvlmResult_t SetGpuIndex(const string &param);

    nvlmResult_t SetLinkIndex(const string& param);

    nvlmResult_t SetCounterIndex(const string& param);

    nvlmResult_t SetSampleNum(const char* param);

    nvlmResult_t SetSampleInterval(const char* param);

    nvlmResult_t SetCounterControl(const string &param);

  private:
    nvlmResult_t SetParams(vector<unsigned int> *vec, const string &param);

    nvlmResult_t SetParams(int *sample_num, const char *user_param);

    nvlmResult_t SetParams(const string &param);

    vector<unsigned int> index_;
    vector<unsigned int> link_;
    vector<unsigned int> counter_;
    int sample_num_;
    int sample_interval_;
    nvmlNvLinkUtilizationControl_t control_;
    unsigned int device_count_;
#ifdef DATABASE
    MyDataBase db;
#endif
};

}  // namespace nvlm

#endif  // _NVLM_UTIL_H_
