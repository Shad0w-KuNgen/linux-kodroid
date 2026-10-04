# deps.cmake — üçüncü taraf bağımlılıklar.
#
# Masaüstü (Linux): sistem paketleri (pkg-config). Android: kaynak koddan FetchContent ile
# derlenir (OpenKO'nun kullandığı çatallar). Her iki durumda da şu hedefler sağlanır:
#   ko::fmt ko::spdlog ko::mpg123 ko::openal ko::freetype ko::jpeg ko::sdl2
# ve KO_SPDLOG_EXTERNAL_FMT (sistem spdlog harici fmt ile derlenmişse ON).

include(FetchContent)

if(ANDROID)
  set(KO_SPDLOG_EXTERNAL_FMT OFF)

  # spdlog (paketli fmt ile; OpenKO <spdlog/fmt/bundled/format.h> kullanır)
  FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/Open-KO/spdlog.git GIT_TAG v1.15.3b-OpenKO GIT_SHALLOW ON)
  set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(spdlog)
  add_library(ko::spdlog ALIAS spdlog)
  add_library(ko_fmt_iface INTERFACE)
  target_link_libraries(ko_fmt_iface INTERFACE spdlog)
  add_library(ko::fmt ALIAS ko_fmt_iface)

  # mpg123
  FetchContent_Declare(mpg123 GIT_REPOSITORY https://github.com/Open-KO/mpg123.git GIT_TAG v1.33.4-dev GIT_SHALLOW ON SOURCE_SUBDIR ports/cmake)
  set(BUILD_LIBOUT123 OFF CACHE BOOL "" FORCE)
  set(BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(mpg123)
  # Port'un hedefi mpg123.h'yi (src/include) dışa açmıyor; OpenKO gibi sarmalayıcı ile ekle
  add_library(ko_mpg123_iface INTERFACE)
  target_link_libraries(ko_mpg123_iface INTERFACE libmpg123)
  target_include_directories(ko_mpg123_iface INTERFACE
    ${mpg123_SOURCE_DIR}/src/include
    ${mpg123_BINARY_DIR}/src/libmpg123
    $<TARGET_PROPERTY:libmpg123,INTERFACE_INCLUDE_DIRECTORIES>)
  add_library(ko::mpg123 ALIAS ko_mpg123_iface)

  # openal-soft (Android: OpenSL backend)
  FetchContent_Declare(openalsoft GIT_REPOSITORY https://github.com/Open-KO/openal-soft.git GIT_TAG 1.25.0-32f75f GIT_SHALLOW ON)
  set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
  set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(ALSOFT_INSTALL OFF CACHE BOOL "" FORCE)
  set(LIBTYPE STATIC CACHE STRING "" FORCE)
  FetchContent_MakeAvailable(openalsoft)
  add_library(ko::openal ALIAS OpenAL)

  # libjpeg
  FetchContent_Declare(libjpeg GIT_REPOSITORY https://github.com/Open-KO/jpeg-cmake.git GIT_TAG v1.3.0a-OpenKO GIT_SHALLOW ON)
  set(LIBJPEG_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  set(LIBJPEG_BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
  set(LIBJPEG_INSTALL OFF CACHE BOOL "" FORCE)
  set(LIBJPEG_BUILD_EXECUTABLES OFF CACHE BOOL "" FORCE)
  set(LIBJPEG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(libjpeg)
  add_library(ko::jpeg ALIAS jpeg_static)

  # FreeType
  FetchContent_Declare(freetype GIT_REPOSITORY https://github.com/freetype/freetype.git GIT_TAG VER-2-13-3 GIT_SHALLOW ON)
  set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
  set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
  set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
  set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
  set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(freetype)
  add_library(ko::freetype ALIAS freetype)

  # SDL2 (Android: libSDL2.so + Java tarafı mobile/android/app/src/main/java/org/libsdl/app)
  FetchContent_Declare(sdl2 GIT_REPOSITORY https://github.com/libsdl-org/SDL.git GIT_TAG release-2.30.10 GIT_SHALLOW ON)
  set(SDL_SHARED ON CACHE BOOL "" FORCE)
  set(SDL_STATIC OFF CACHE BOOL "" FORCE)
  set(SDL_TEST OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(sdl2)
  add_library(ko::sdl2 ALIAS SDL2)
else()
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(FMT REQUIRED IMPORTED_TARGET fmt)
  pkg_check_modules(SPDLOG REQUIRED IMPORTED_TARGET spdlog)
  pkg_check_modules(MPG123 REQUIRED IMPORTED_TARGET libmpg123)
  pkg_check_modules(OPENAL REQUIRED IMPORTED_TARGET openal)
  pkg_check_modules(FREETYPE REQUIRED IMPORTED_TARGET freetype2)
  pkg_check_modules(JPEG REQUIRED IMPORTED_TARGET libjpeg)
  pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
  add_library(ko::fmt ALIAS PkgConfig::FMT)
  add_library(ko::spdlog ALIAS PkgConfig::SPDLOG)
  add_library(ko::mpg123 ALIAS PkgConfig::MPG123)
  add_library(ko::openal ALIAS PkgConfig::OPENAL)
  add_library(ko::freetype ALIAS PkgConfig::FREETYPE)
  add_library(ko::jpeg ALIAS PkgConfig::JPEG)
  add_library(ko::sdl2 ALIAS PkgConfig::SDL2)
  # Sistem spdlog'u harici fmt kullanıyorsa <spdlog/fmt/bundled/...> yolu için shim gerekir
  set(KO_SPDLOG_EXTERNAL_FMT ON)
  if(EXISTS "/usr/include/spdlog/fmt/bundled/format.h")
    set(KO_SPDLOG_EXTERNAL_FMT OFF)
  endif()
endif()
