cmake_minimum_required(VERSION 3.12)
project(minichlink)

set(CMAKE_C_STANDARD 11)

set(C_S minichlink.c pgm-wch-linke.c pgm-esp32s2-ch32xx.c nhc-link042.c minichgdb.c)

# only for macOS branch...
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
set(CFLAGS "-Wall -Wno-asm-operand-widths -Wno-deprecated-declarations -Wno-deprecated-non-prototype")
find_package(PkgConfig REQUIRED)
find_library(COREFOUNDATION_LIBRARY CoreFoundation)
find_library(IOKIT_LIBRARY IOKit)
pkg_check_modules(LIBUSB REQUIRED IMPORTED_TARGET libusb-1.0)
add_compile_definitions(__MACOSX__)

add_executable(minichlink ${C_S})
target_link_libraries(minichlink PkgConfig::LIBUSB Threads::Threads ${COREFOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
target_compile_options(minichlink PRIVATE ${CFLAGS})
