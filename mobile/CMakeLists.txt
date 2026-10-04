# Knight Online Mobile — OpenKO (1.298) istemcisinin Android/Linux (SDL2 + OpenGL ES 3) portu.
#
# Katmanlar:
#   compat/win32      Win32/DirectInput/Winsock uyumluluk başlıkları
#   d3d9gles          Direct3D 9 → OpenGL ES 3 çeviri katmanı
#   engine-port       N3Base'in Windows'a bağlı parçalarının taşınabilir sürümleri
#   openko/src        OpenKO kaynak kodu (git subtree, MIT)
#   platform          SDL2 uygulama kabuğu (WinMain yerine), dokunmatik girdi
cmake_minimum_required(VERSION 3.16)
project(KnightOnlineMobile CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(OPENKO_SRC "${CMAKE_CURRENT_SOURCE_DIR}/openko/src")
set(KO_COMPAT  "${CMAKE_CURRENT_SOURCE_DIR}/compat")

option(KO_BUILD_TESTS "Birim testlerini derle" ON)

include(cmake/deps.cmake)

# --- d3d9gles --------------------------------------------------------------
set(D3D9GLES_BUILD_TESTS ${KO_BUILD_TESTS})
add_subdirectory(d3d9gles)

# --- compat ----------------------------------------------------------------
add_library(ko_compat STATIC compat/win32/windows.cpp compat/win32/registry.cpp compat/win32/findfile.cpp compat/win32/ini.cpp compat/win32/winsock.cpp compat/win32/paths.cpp)
target_include_directories(ko_compat PUBLIC ${KO_COMPAT}/win32)
if(KO_SPDLOG_EXTERNAL_FMT)
  target_include_directories(ko_compat PUBLIC ${KO_COMPAT}/spdlog-bundled)
endif()
if(ANDROID)
  target_link_libraries(ko_compat PUBLIC log)
endif()

# OpenKO kaynakları için ortak derleme ayarları
function(ko_openko_target tgt)
  target_compile_definitions(${tgt} PUBLIC _N3GAME NOMINMAX _CRT_SECURE_NO_WARNINGS KO_MOBILE=1)
  target_include_directories(${tgt} PUBLIC ${OPENKO_SRC})
  target_link_libraries(${tgt} PUBLIC ko_compat ko::fmt ko::spdlog)
  if(NOT WIN32)
    target_compile_options(${tgt} PRIVATE -include ${KO_COMPAT}/win32/ko_fopen.h)
  endif()
  target_compile_options(${tgt} PRIVATE -Wall -Wno-unused-parameter -Wno-unused-variable -Wno-sign-compare
    -Wno-unused-but-set-variable -Wno-misleading-indentation -Wno-parentheses -Wno-reorder -Wno-switch
    -Wno-deprecated-declarations -Wno-unknown-pragmas -Wno-missing-field-initializers
    $<$<CXX_COMPILER_ID:GNU>:-Wno-changes-meaning>)
endfunction()

# --- shared ----------------------------------------------------------------
add_library(ko_shared STATIC
  ${OPENKO_SRC}/shared/ByteBuffer.cpp
  ${OPENKO_SRC}/shared/CircularBuffer.cpp
  ${OPENKO_SRC}/shared/crc32.cpp
  ${OPENKO_SRC}/shared/DateTime.cpp
  ${OPENKO_SRC}/shared/ExpandableCircularBuffer.cpp
  ${OPENKO_SRC}/shared/globals.cpp
  ${OPENKO_SRC}/shared/Ini.cpp
  ${OPENKO_SRC}/shared/JvCryption.cpp
  ${OPENKO_SRC}/shared/lzf.cpp
  ${OPENKO_SRC}/shared/StringUtils.cpp
  ${OPENKO_SRC}/shared/Thread.cpp
  ${OPENKO_SRC}/shared/TimerThread.cpp
)
ko_openko_target(ko_shared)
target_precompile_headers(ko_shared PRIVATE ${OPENKO_SRC}/shared/pch.h)
set_source_files_properties(${OPENKO_SRC}/shared/crc32.cpp ${OPENKO_SRC}/shared/lzf.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

# --- FileIO / MathUtils ------------------------------------------------------
add_library(ko_fileio STATIC
  engine-port/FileReader_posix.cpp   # Boost yerine POSIX mmap
  ${OPENKO_SRC}/FileIO/FileWriter.cpp
)
ko_openko_target(ko_fileio)

add_library(ko_mathutils STATIC ${OPENKO_SRC}/MathUtils/MathUtils.cpp)
ko_openko_target(ko_mathutils)

# --- N3Base (oyun motoru) -----------------------------------------------------
file(GLOB N3BASE_ALL_SOURCES CONFIGURE_DEPENDS ${OPENKO_SRC}/N3Base/*.cpp)
set(N3BASE_EXCLUDE
  N3EngTool.cpp N3PMeshCreate.cpp Pick.cpp N3FXGroup.cpp N3GlobalEffectMng.cpp # yalnızca araçlar
  DFont.cpp      # → engine-port/DFont_ft.cpp (FreeType)
  WinCrypt.cpp   # → engine-port/WinCrypt_portable.cpp (SHA-1 + RC4)
)
set(N3BASE_SOURCES)
foreach(src ${N3BASE_ALL_SOURCES})
  get_filename_component(name ${src} NAME)
  if(NOT name IN_LIST N3BASE_EXCLUDE)
    list(APPEND N3BASE_SOURCES ${src})
  endif()
endforeach()

add_library(ko_n3base STATIC
  ${N3BASE_SOURCES}
  engine-port/WinCrypt_portable.cpp
  engine-port/DFont_ft.cpp
  engine-port/N3UIEdit_portable.cpp
)
ko_openko_target(ko_n3base)
target_include_directories(ko_n3base PUBLIC ${OPENKO_SRC}/N3Base)
target_link_libraries(ko_n3base PUBLIC ko_fileio ko_mathutils ko_shared d3d9gles
  ko::mpg123 ko::openal ko::freetype)
target_precompile_headers(ko_n3base PRIVATE ${OPENKO_SRC}/N3Base/StdAfxBase.h)

# --- JpegFile ------------------------------------------------------------------
add_library(ko_jpegfile STATIC ${OPENKO_SRC}/Client/JpegFile/JpegFile.cpp engine-port/JpegFile_portable.cpp)
ko_openko_target(ko_jpegfile)
target_include_directories(ko_jpegfile PUBLIC ${OPENKO_SRC}/Client)
target_link_libraries(ko_jpegfile PUBLIC ko::jpeg d3d9gles)

# --- WarFare (oyun istemcisi) --------------------------------------------------
set(WARFARE_DIR ${OPENKO_SRC}/Client/WarFare)
set(WARFARE_CONFIG_DIR ${CMAKE_BINARY_DIR}/WarFare)
file(MAKE_DIRECTORY ${WARFARE_CONFIG_DIR})
if(NOT EXISTS ${WARFARE_CONFIG_DIR}/warfare_config.h)
  configure_file(${WARFARE_DIR}/warfare_config.h.default ${WARFARE_CONFIG_DIR}/warfare_config.h COPYONLY)
endif()
file(GLOB WARFARE_ALL_SOURCES CONFIGURE_DEPENDS ${WARFARE_DIR}/*.cpp)
set(WARFARE_EXCLUDE
  WarFareMain.cpp  # → platform/main_sdl.cpp
  LocalInput.cpp   # → platform/LocalInput_sdl.cpp
)
set(WARFARE_SOURCES)
foreach(src ${WARFARE_ALL_SOURCES})
  get_filename_component(name ${src} NAME)
  if(NOT name IN_LIST WARFARE_EXCLUDE)
    list(APPEND WARFARE_SOURCES ${src})
  endif()
endforeach()
add_library(ko_warfare STATIC ${WARFARE_SOURCES})
ko_openko_target(ko_warfare)
target_include_directories(ko_warfare PUBLIC ${WARFARE_DIR} ${WARFARE_CONFIG_DIR})
target_link_libraries(ko_warfare PUBLIC ko_n3base ko_jpegfile)
target_precompile_headers(ko_warfare PRIVATE ${WARFARE_DIR}/StdAfx.h)

# --- Uygulama (SDL2 kabuğu) ---------------------------------------------------
set(KO_APP_SOURCES
  platform/main_sdl.cpp
  platform/LocalInput_sdl.cpp
  platform/KoPlatformInput.cpp
)
if(ANDROID)
  # Android'de uygulama bir paylaşılan kütüphanedir; SDLActivity libKnightOnLine.so içindeki SDL_main'i çağırır
  add_library(KnightOnLine SHARED ${KO_APP_SOURCES})
  target_link_libraries(KnightOnLine PRIVATE ko_warfare ko::sdl2 GLESv3 EGL android log)
else()
  add_executable(KnightOnLine ${KO_APP_SOURCES})
  target_link_libraries(KnightOnLine PRIVATE ko_warfare ko::sdl2 GLESv2 EGL)
endif()
ko_openko_target(KnightOnLine)
target_include_directories(KnightOnLine PRIVATE platform)

# --- Testler -----------------------------------------------------------------
if(KO_BUILD_TESTS AND NOT ANDROID)
  enable_testing()
  add_executable(ko_crypt_test engine-port/tests/crypt_test.cpp engine-port/WinCrypt_portable.cpp)
  target_link_libraries(ko_crypt_test PRIVATE ko_n3base)
  add_test(NAME ko_crypt_test COMMAND ko_crypt_test)
endif()
