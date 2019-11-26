#include "util.h"

namespace nvlm {

Operations::Operations() {
  counter_ = { 0 };
  sample_num_ = 10;
  sample_interval_ = 1;
  control_.units = NVML_NVLINK_COUNTER_UNIT_BYTES;
  control_.pktfilter = NVML_NVLINK_COUNTER_PKTFILTER_ALL;
#ifdef DATABASE
  db.connect("10.101.16.128", "root", "123456", "nvlink", 33066);
  cout << "database connect OK." << endl;
#endif
  NV_CHECK(nvmlInit());
  NV_CHECK(nvmlDeviceGetCount(&device_count_));
  for (int i = 0; i < device_count_; ++i) {
    index_.push_back(i);
  }
  for (int i = 0; i < NVML_NVLINK_MAX_LINKS; ++i) {
    link_.push_back(i);
  }
}

nvlmResult_t Operations::GetRemotePciInfo(int index, 
                                          unsigned int link, 
                                          nvmlPciInfo_t &info) {

  NV_CHECK(nvmlDeviceGetHandleByIndex(index, &device));
  NV_CHECK(nvmlDeviceGetNvLinkRemotePciInfo(device, link, &info));

  return NVLM_SUCCESS;
}

nvlmResult_t Operations::FillTopoMatrix() {
  nvmlPciInfo_t info[MAX_DEVICE_COUNT], tmp_info;

  for (int i = 0; i < device_count_; ++i) {
    NV_CHECK(nvmlDeviceGetHandleByIndex(i, &device));
    NV_CHECK(nvmlDeviceGetPciInfo(device, &info[i]));
  }

  for (int i = 0; i < device_count_; ++i) {
    for (int j = 0; j < NVML_NVLINK_MAX_LINKS; ++j) {
      GetRemotePciInfo(i, j, tmp_info);
      for (int k = 0; k < device_count_; ++k) {
        if (!strcmp(info[k].busId, tmp_info.busId)) {
          topo[i][j] = k;
          break;
        }
      }
    }
  }

  return NVLM_SUCCESS;
}

nvlmResult_t Operations::GetGpuUtilRates() {
  nvmlUtilization_t utilization;

  for (int i = 0; i < sample_num_; i++) {
    for (auto id : index_) {
      NV_CHECK(nvmlDeviceGetHandleByIndex(id, &device));
      NV_CHECK(nvmlDeviceGetUtilizationRates(device, &utilization));
      cout << "gpu_" << id << endl;
      cout << "gpu-util: " << utilization.gpu << "%" << endl;
      cout << "mem-util: " << utilization.memory << "%" << endl;
#ifdef DATABASE
      db.insertitem("test", to_string(utilization.gpu) + ',' + to_string(time(0)));
#endif
    }
    usleep(sample_interval_ * 1000);
  }

  return NVLM_SUCCESS;
}

nvlmResult_t Operations::GetLinkStatus(void) {
  nvmlEnableState_enum status;
  for (auto index : index_) {
    NV_CHECK(nvmlDeviceGetHandleByIndex(index, &device));
    NV_CHECK(nvmlDeviceGetName(device, name, NVML_DEVICE_NAME_BUFFER_SIZE));
    cout << "device" << index << ": " << name << endl;
    for (auto link : link_) {
      NV_CHECK(nvmlDeviceGetNvLinkState(device, link, &status));
      cout << "link" << link << ": "
           << ((status == NVML_FEATURE_ENABLED) ? "active" : "inactive") << endl;
    }
    cout << "-----------------------------" << endl;
  }

  return NVLM_SUCCESS;
}

nvlmResult_t Operations::GetLinkBandwidth(void) {
  float util_r, util_t, bw;
  uint64_t ms0 = 0, ms1 = 1;
  unsigned long long rx0 = 0, rx1 = 0, tx0 = 0, tx1 = 0;
  ofstream bw_file;

  for (auto id : index_) {
    NV_CHECK(nvmlDeviceGetHandleByIndex(id, &device));
    NV_CHECK(nvmlDeviceGetName(device, name, NVML_DEVICE_NAME_BUFFER_SIZE));
    cout << "device" << id << ": " << name << endl;
    for (auto link : link_) {
      for (auto counter : counter_) {
        bw_file.open("../file/GPU"+to_string(id)+"_Link"+to_string(link)+"_"+to_string(time(0))+".csv", ios::out);
        bw_file << "time_stamp, bandwidth\n";
        NV_CHECK(nvmlDeviceSetNvLinkUtilizationControl(device, link, counter, &control_, NVLM_RESET));
        for (int i = 0; i < sample_num_; i++) {
          ms1 = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
          NV_CHECK(nvmlDeviceGetNvLinkUtilizationCounter(device, link, counter, &rx1, &tx1));
          bw = (1.0f/((ms1-ms0)/1000.0f))*(((rx1-rx0)+(tx1-tx0))/1024.0f/1024.0f/1024.0f);
          if (bw != 0) {
            cout << "tmie stamp: " << ms1 << endl
                 << "link: " << link << "  counter: " << counter << endl
                 << "rx1:  " << rx1 << "(Bytes)" << endl
                 << "tx1:  " << tx1 << "(Bytes)" << endl
                 << "real-time bandwidth: " << bw << "(GB/s)" << endl
                 << "----------------------------------------" << endl;
            bw_file << ms1 << ","<< bw << "\n";
          }
#ifdef DATABASE
          db.insertitem("test2", to_string(bw) + ',' + to_string(time(0)));
          // grafana only supports "second".
#endif
          rx0 = rx1;
          tx0 = tx1;
          ms0 = ms1;
          usleep(sample_interval_ * 1000);  // us
        }
        bw_file.close();
      }
    }
  }

  return NVLM_SUCCESS;
}

nvlmResult_t GetLinkSetBandwidth() {
  float bw, max_dw = 0;
  uint64_t ms0 = 0, ms1 = 1;
  unsigned long long data_size = 0;
  ofstream bw_file;

  for (auto id : index_) {
    NV_CHECK(nvmlDeviceGetHandleByIndex(id, &device));
    for (auto link : link_) {
      for (auto counter : counter_) {
        NV_CHECK(nvmlDeviceSetNvLinkUtilizationControl(device, link, counter, &control_, NVLM_RESET));
      }
    }
  }

  vector<struct Link> link_set;
  struct Link tmp_link;
  for (int i = 0; i < link_.size(); i++) {
    tmp_link.rank = link_[i];
    link_set.push_back(tmp_link);
  }

  for (auto id : index_) {
    NV_CHECK(nvmlDeviceGetHandleByIndex(id, &device));
    for (auto counter : counter_) {
      bw_file.open("../file/GPU"+to_string(id)+"_"+to_string(time(0))+".csv", ios::out);
      bw_file << "time_stamp, bandwidth\n";
      for (int i = 0; i < sample_num_; i++) {
        ms1 = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        for (auto& link : link_set) {
          NV_CHECK(nvmlDeviceGetNvLinkUtilizationCounter(device, link.rank, counter, &link.rx1, &link.tx1));
          
          data_size += (link.rx1 - link.rx0) + (link.tx1 - link.tx0);
          if (data_size != 0) {
            // cout << "data_size: " << data_size << endl;
          }
          link.rx0 = link.rx1;
          link.tx0 = link.tx1;
        }
        bw = (1.0f/((ms1-ms0)/1000.0f))*(data_size/1024.0f/1024.0f/1024.0f);
        if (bw != 0) {
            cout << "tmie stamp: " << ms1 << endl
                 << "data_size:  " << data_size << "(Bytes)" << endl
                 << "real-time bandwidth: " << bw << "(GB/s)" << endl
                 << "----------------------------------------" << endl;
            bw_file << ms1 << ","<< bw << "\n";
        }
        // if (bw > max_bw) {
        //  max_bw = bw;
        //  cout << "max_bw: " << max_bw << "  time_stamp: " << ms1 << endl;
        //}

        ms0 = ms1;
        data_size = 0;

        usleep(sample_interval_ * 1000);
      }
      bw_file.close();
    }
  }

  return NVLM_SUCCESS;
}

nvlmResult_t Operations::GetNvlinkTopo() {
  NVLM_CHECK(FillTopoMatrix());

  for (int i = 0; i < device_count_; i++) {
    cout << endl << "GPU[" << i << "]:  ";
    for (int j = 0; j < NVML_NVLINK_MAX_LINKS; j++) {
      cout << "C" << j << "->G" << topo[i][j] << "  ";
    }
    cout << endl;
  }
  cout << endl << "'C' means channel, 'G' means GPU." << endl;

  return NVLM_SUCCESS;
}

nvlmResult_t Operations::SetGpuIndex(const string& param) {
  for (auto index : param) {
    if ((index - '0') >= device_count_) {
      return NVLM_FAIL;
    }
  }

  SetParams(&index_, param);
  return NVLM_SUCCESS;
}

nvlmResult_t Operations::SetLinkIndex(const string& param) {
  for (auto index : param) {
    if ((index - '0') >=  NVML_NVLINK_MAX_LINKS || (index - '0') < 0) {
      return NVLM_FAIL;
    }
  }
  SetParams(&link_, param);
  return NVLM_SUCCESS;
}

nvlmResult_t Operations::SetCounterIndex(const string& param) {
  for (auto index : param) {
    if ((index - '0') > 1 || (index - '0') < 0) {
      return  NVLM_FAIL;
    }
  }
  SetParams(&counter_, param);
  return NVLM_SUCCESS;
}

nvlmResult_t Operations::SetCounterControl(const string &param) {
    for (auto it : param) {
      if (it != 'c' && it != 'p' && it != 'b' && it != 'n' && it != 'r' &&
          it != 'w' && it != 'x' && it != 'y' && it != 'f' && it != 'd' &&
          it != 'o' && it != 'z') {
        return NVLM_FAIL;
      }
    }
  SetParams(param);
  return NVLM_SUCCESS;
}

nvlmResult_t Operations::SetSampleNum(const char* param) {
  if (atoi(param) < 0) {
    return NVLM_FAIL;
  }
  SetParams(&sample_num_, param);
  return NVLM_SUCCESS;
}

nvlmResult_t Operations::SetSampleInterval(const char* param) {
  if (atoi(param) < 0) {
    return NVLM_FAIL;
  }
  SetParams(&sample_interval_, param);
  return NVLM_SUCCESS;
}

nvlmResult_t Operations::SetParams(vector<unsigned int> *vec, 
                                   const string &param) {
  vec->clear();
  for (int i = 0; i < param.size(); i++) {
    vec->push_back(param[i] - '0');
  }

  return NVLM_SUCCESS;
}

nvlmResult_t Operations::SetParams(int *num, const char *param) {
  *num = atoi(param);

  return NVLM_SUCCESS;
}

nvlmResult_t Operations::SetParams(const string &param) {
  for (int i = 0; i < param.size(); i++) {
    switch (param[i]) {
      case 'c':
        control_.units = NVML_NVLINK_COUNTER_UNIT_CYCLES;
        break;
      case 'p':
        control_.units = NVML_NVLINK_COUNTER_UNIT_PACKETS;
        break;
      case 'b':
        control_.units = NVML_NVLINK_COUNTER_UNIT_BYTES;
        break;
      case 'n':
        control_.pktfilter = NVML_NVLINK_COUNTER_PKTFILTER_NOP;
        break;
      case 'r':
        control_.pktfilter = NVML_NVLINK_COUNTER_PKTFILTER_READ;
        break;
      case 'w':
        control_.pktfilter = NVML_NVLINK_COUNTER_PKTFILTER_WRITE;
        break;
      case 'x':
        control_.pktfilter = NVML_NVLINK_COUNTER_PKTFILTER_RATOM;
        break;
      case 'y':
        control_.pktfilter = NVML_NVLINK_COUNTER_PKTFILTER_NRATOM;
        break;
      case 'f':
        control_.pktfilter = NVML_NVLINK_COUNTER_PKTFILTER_FLUSH;
        break;
      case 'd':
        control_.pktfilter = NVML_NVLINK_COUNTER_PKTFILTER_RESPDATA;
        break;
      case 'o':
        control_.pktfilter = NVML_NVLINK_COUNTER_PKTFILTER_RESPNODATA;
        break;
      case 'z':
        control_.pktfilter = NVML_NVLINK_COUNTER_PKTFILTER_ALL;
        break;
      default:
        cout << "set counter control error." << endl;
        break;
    }
  }

  return NVLM_SUCCESS;
}

nvlmResult_t Operations::GetHelp() {
  cout << "options: \n"
       << "-h,  --help                 Display help information.\n"
       << "-i,  --id                   Set GPU index, default is \"0123\".\n"
       << "-l,  --link                 Set link index, default is \"012345\".\n"
       << "-s,  --linkstatus           Display link state (active/inactive).\n"
       << "-o,  --topo                 Get global nvlink topology.\n"
       << "-v,  --sampleinterval       Set sample interval, default is 1ms.\n"
       << "-n,  --samplenum            Set sample times, default is 10 times.\n"
       // << "-p,  --savepath             Set output file saving path\n"
       // << "-f,  --fileformat           Set output file format.\n"
       << "-u,  --utilization          Get utilization information, includeing"
       << " gpu utilization and memory utilization.\n"
       // << "-r,  --resetcounters        Reset link utilization counter for"
       // << "specified counter set (0 or 1).\n"
       // << "-e,  --errorcounters        Display error counters for a link.\n"
       // << "-ec, --crcerrorcounters     Display per-lane CRC error counters for"
       // << "a link.\n"
       // << "-re, --reseterrorcounters   Reset all error counters to zero.\n"
       << "-b,  --linkbandwidth        Get real-tmie link bandwidth (GB/s).\n"
       << "-w,  --linksetbandwidth     Get real-tmie link-set bandwidth (GB/s).\n"
       << "-c,  --counterindex         Set counter index, default is 0.\n"
       << "-k,  --setcontrol           Set the utilization counters to count"
       << "specific NvLink transactions, default is \"bz\".\n"
       << "\t\t\t    The argument consists of an 2-character string representing"
       << "what is meant to be counted:\n"
       // << "\t\t\t    First character specifies the counter set:\n"
       // << "\t\t\t     0 = counter 0\n"
       // << "\t\t\t     1 = counter 1\n"
       << "\t\t\t    First character can be:\n"
       << "\t\t\t     c = count cycles\n"
       << "\t\t\t     p = count packets\n"
       << "\t\t\t     b = count bytes\n"
       << "\t\t\t    Second character can be any of the following:\n"
       << "\t\t\t     n = nop\n"
       << "\t\t\t     r = read\n"
       << "\t\t\t     w = write\n"
       << "\t\t\t     x = reduction atomic requests\n"
       << "\t\t\t     y = non-reduction atomic requests\n"
       << "\t\t\t     f = flush\n"
       << "\t\t\t     d = responses with data\n"
       << "\t\t\t     o = responses with no data\n"
       << "\t\t\t     z = all traffic\n";

  return NVLM_SUCCESS;
}

}  // namespace nvlm
