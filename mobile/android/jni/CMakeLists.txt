# Android NDK girişi: mobile/ projesini ANDROID=ON ile derler.
# Bağımlılıklar (SDL2, openal-soft, mpg123, libjpeg, FreeType, spdlog) FetchContent ile
# kaynak koddan derlenir (mobile/cmake/deps.cmake); ilk derleme ağ erişimi ister.
cmake_minimum_required(VERSION 3.22)
project(KnightOnlineMobileAndroid CXX C)
set(KO_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_BINARY_DIR}/mobile)
