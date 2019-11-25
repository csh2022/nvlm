## Build
```sh
cd nvlm
mkdir build
cd build
cmake ..
make
```

## Run
```sh
../bin/nvlm -i 0 -l 3 -c 0 -v 1 -n 10000 -k bz -b
../bin/nvlm --id 0 --link 3 --counter 0 --interval 1 --samplenum 10000 -countercontrol bz --linkbandwidth 
```