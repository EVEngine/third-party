
mkdir -p build 
cd build

cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target install -j `nproc`

LAST_MSG = `git log -1 --format=%s`

if [[ $OSTYPE == 'darwin'* ]]; then
    git checkout darwin 
    git add ./binary
    git commit -m "$LAST_MSG"
    git checkout main
else 
    git checkout linux 
    git add ./binary
    git commit -m "$LAST_MSG"
    git checkout main
fi