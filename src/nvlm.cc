// #include "nvlm.h"
#include "util.h"

using namespace nvlm;

int main(int argc, char* argv[]) {
  // google::InitGoogleLogging(argv[0]);
  // google::SetLogDestination(google::GLOG_INFO, "../log/");

  int c;
  int digit_optind = 0;
  Operations op;

  while (true) {
    int this_option_optind = optind ? optind : 1;
    int option_index = 0;
    static struct option long_options[] = {
      { "help",             no_argument,       0, 'h' },
      { "id",               required_argument, 0, 'i' },
      { "link",             required_argument, 0, 'l' },
      { "linkstatus",       no_argument,       0, 's' },
      { "topo",             no_argument,       0, 'o' },
      { "interval",         required_argument, 0, 'v' },
      { "samplenum",        required_argument, 0, 'n' },
      { "utilrate",         no_argument,       0, 'u' },
      { "counter",          required_argument, 0, 'c' },
      { "countercontrol",   required_argument, 0, 'k' },
      { "linkbandwidth",    no_argument,       0, 'b' },
      { "linksetbandwidth", no_argument,       0, 'w' },
      // { "savepath",         required_argument, 0, 'p' },
      // { "fileformat",       required_argument, 0, 'f' },
      // { "resetcounter",     no_argument,       0, 'r' },
      // { "errorcounter",     no_argument,       0, 'e' },
      // { "crcerrorcounter",  no_argument,       0, 'c' },
      // { "reseterrorcounter",no_argument,       0, 't' },
      { 0, 0, 0, 0 }
    };
    c = getopt_long(argc, argv, "hi:l:sov:n:uc:k:b", long_options, &option_index);
    if (c == -1) break;
    switch (c) {
      case 'h':
        NVLM_CHECK(op.GetHelp());
        break;
      case 'i':
        NVLM_CHECK(op.SetGpuIndex(optarg));
        break;
      case 'l':
        NVLM_CHECK(op.SetLinkIndex(optarg));
        break;
      case 's':
        NVLM_CHECK(op.GetLinkStatus());
        break;
      case 'o':
        NVLM_CHECK(op.GetNvlinkTopo());
        break;
      case 'v':
        NVLM_CHECK(op.SetSampleInterval(optarg));
        break;
      case 'n':
        NVLM_CHECK(op.SetSampleNum(optarg));
        break;
      case 'u':
        NVLM_CHECK(op.GetGpuUtilRates());
        break;
      case 'c':
        NVLM_CHECK(op.SetCounterIndex(optarg));
        break;
      case 'k':
        NVLM_CHECK(op.SetCounterControl(optarg));
        break;
      case 'b':
        NVLM_CHECK(op.GetLinkBandwidth());
        break;
      case 'w':
        NVLM_CHECK(op.GetLinkSetBandwidth());
        break;
      default:
        NVLM_CHECK(op.GetHelp());
        exit(EXIT_FAILURE);
    }
  }

  return 0;
}
