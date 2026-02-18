if [[ ! -d build ]]; then
    mkdir build
fi

pushd build

clang -o imm_opts -Wall --std=c23 -g -Wno-unused-function -fdiagnostics-absolute-paths ../main.c

popd
