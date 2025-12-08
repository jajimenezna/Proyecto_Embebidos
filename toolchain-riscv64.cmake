set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

set(TOOLCHAIN_PATH "$ENV{HOME}/Downloads/maixcam_toolchain/host-tools/gcc/riscv64-linux-musl-x86_64")
set(CMAKE_C_COMPILER "${TOOLCHAIN_PATH}/bin/riscv64-unknown-linux-musl-gcc")

set(CMAKE_C_FLAGS "-march=rv64imafdcv -mabi=lp64d" CACHE STRING "")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
