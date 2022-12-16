mkdir -p build 
cd build

cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target install -j `nproc`

mkdir -p ../build-mloader
cd ../build-mloader
cmake -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=../../third-party-binary \
    -DDEP_PATH=`pwd`/../../third-party-binary ../medialoader
cmake --build . --target install -j `nproc`

