cmake_minimum_required(VERSION 3.16)
project(nrphy CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# SIMD: NEON is the default on arm64; add AVX2 baseline on x86.
# AVX-512 / SVE / AMX intrinsics are isolated per-target in bench/ (see chapter 07).
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
  add_compile_options(-mavx2)
endif()

add_library(nrphy
  src/crc.cpp
  src/gold.cpp
  src/scrambler.cpp
  src/modulator.cpp
  src/layer_map.cpp
  src/transform_precoder.cpp
  src/ldpc.cpp
  src/rate_match.cpp
  src/segmentation.cpp
  src/precode.cpp
  src/re_map.cpp
  src/pusch.cpp
)

add_executable(pusch_tx_demo src/pusch_tx_demo.cpp)
target_link_libraries(pusch_tx_demo PRIVATE nrphy)

foreach(b scrambler modulator crc)
  add_executable(${b}_bench bench/${b}_bench.cpp)
  target_link_libraries(${b}_bench PRIVATE nrphy)
endforeach()
target_include_directories(nrphy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

enable_testing()

foreach(t crc gold scrambler modulator layer_map transform_precoder ldpc rate_match segmentation)
  add_executable(test_${t} test/test_${t}.cpp)
  target_link_libraries(test_${t} PRIVATE nrphy)
  add_test(NAME ${t} COMMAND test_${t})
endforeach()
