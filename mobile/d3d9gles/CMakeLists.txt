# d3d9gles — Direct3D 9 sabit işlev API'sinin OpenGL ES 3.0 üzerinde uygulanması.
# Knight Online (OpenKO) istemcisinin N3Base motoru değiştirilmeden mobil GPU'larda çalışsın diye.
cmake_minimum_required(VERSION 3.16)
project(d3d9gles CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(d3d9gles STATIC
  src/Device.cpp
  src/Resources.cpp
  src/Shaders.cpp
  src/D3DX.cpp
)

target_include_directories(d3d9gles PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../compat/win32
)

if(ANDROID)
  target_link_libraries(d3d9gles PUBLIC GLESv3 EGL log)
else()
  target_link_libraries(d3d9gles PUBLIC GLESv2 EGL)
endif()

target_compile_options(d3d9gles PRIVATE -Wall -Wextra -Wno-unused-parameter)

option(D3D9GLES_BUILD_TESTS "d3d9gles testlerini derle (başsız EGL, Mesa llvmpipe ile çalışır)" ON)
if(D3D9GLES_BUILD_TESTS AND NOT ANDROID)
  add_executable(d3d9gles_smoke tests/smoke_test.cpp ../compat/win32/windows.cpp)
  target_link_libraries(d3d9gles_smoke PRIVATE d3d9gles)
  enable_testing()
  add_test(NAME d3d9gles_smoke COMMAND d3d9gles_smoke)
  set_tests_properties(d3d9gles_smoke PROPERTIES ENVIRONMENT "EGL_PLATFORM=surfaceless;LIBGL_ALWAYS_SOFTWARE=1")
endif()
