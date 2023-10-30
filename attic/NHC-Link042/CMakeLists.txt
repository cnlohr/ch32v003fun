# CMake minimum version
cmake_minimum_required (VERSION 3.1)

# Project Infomation
project( NHC-Link042 )
enable_language(ASM)
enable_language(C)

# Reset output path
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set (LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

# STDLIB
set (CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)

# Set include path
include_directories (./User)
include_directories (./STM32F0x2_USB-FS-Device_Lib_V1.0.0/Libraries/CMSIS/Device/ST/STM32F0xx/Include)
include_directories (./STM32F0x2_USB-FS-Device_Lib_V1.0.0/Libraries/STM32F0xx_StdPeriph_Driver/inc)
include_directories (./STM32F0x2_USB-FS-Device_Lib_V1.0.0/Libraries/STM32_USB_Device_Driver/inc)
include_directories (./STM32F0x2_USB-FS-Device_Lib_V1.0.0/Libraries/STM32_USB_Device_Library/Core/inc)
include_directories (./LibUSB)
include_directories (./STM32F0x2_USB-FS-Device_Lib_V1.0.0/Libraries/CMSIS/Include)

# The need build source path and build all files
set (SRC_FILE0 ./User/it.c)
set (SRC_FILE1 ./User/main.c)
set (SRC_FILE2 ./User/usb_bsp.c)
set (SRC_FILE3 ./User/usbd_pwr.c)
set (SRC_FILE4 ./User/usbd_desc.c)
set (SRC_FILE5 ./User/usbd_usr.c)
set (SRC_FILE6 ./User/stm32_it.c)
set (SRC_FILE7 ./User/NHC_WCH_SDI.c)
set (SRC_FILE8 ./User/NHC_WCH_SDI.h)
set (SRC_FILE9 ./User/config.h)
set (SRC_FILE10 ./User/usbd_desc.h)
set (SRC_FILE11 ./LibUSB/LibUSB.c)
set (SRC_FILE12 ./User/system_stm32f0xx.c)
set (SRC_FILE13 ./STM32F0x2_USB-FS-Device_Lib_V1.0.0/Libraries/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_crs.c)
set (SRC_FILE14 ./STM32F0x2_USB-FS-Device_Lib_V1.0.0/Libraries/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_exti.c)
set (SRC_FILE15 ./STM32F0x2_USB-FS-Device_Lib_V1.0.0/Libraries/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_gpio.c)
set (SRC_FILE16 ./STM32F0x2_USB-FS-Device_Lib_V1.0.0/Libraries/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_rcc.c)
set (SRC_FILE17 ./STM32F0x2_USB-FS-Device_Lib_V1.0.0/Libraries/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_syscfg.c)
set (SRC_FILE18 ./STM32F0x2_USB-FS-Device_Lib_V1.0.0/Libraries/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_tim.c)
set (SRC_FILE19 ./STM32F0x2_USB-FS-Device_Lib_V1.0.0/Libraries/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_misc.c)
set (SRC_FILE20 ./STM32F0x2_USB-FS-Device_Lib_V1.0.0/Libraries/STM32_USB_Device_Driver/src/usb_core.c)
set (SRC_FILE21 ./STM32F0x2_USB-FS-Device_Lib_V1.0.0/Libraries/STM32_USB_Device_Driver/src/usb_dcd.c)
set (SRC_FILE22 ./STM32F0x2_USB-FS-Device_Lib_V1.0.0/Libraries/STM32_USB_Device_Driver/src/usb_dcd_int.c)
set (SRC_FILE23 ./STM32F0x2_USB-FS-Device_Lib_V1.0.0/Libraries/STM32_USB_Device_Library/Core/src/usbd_core.c)
set (SRC_FILE24 ./STM32F0x2_USB-FS-Device_Lib_V1.0.0/Libraries/STM32_USB_Device_Library/Core/src/usbd_ioreq.c)
set (SRC_FILE25 ./STM32F0x2_USB-FS-Device_Lib_V1.0.0/Libraries/STM32_USB_Device_Library/Core/src/usbd_req.c)


set(CROSS_TARGET_TRIPLET "arm-none-eabi-")

# CC AR LD AS
set(CMAKE_C_COMPILER "${CROSS_TARGET_TRIPLET}gcc")
set(CMAKE_ASM_COMPILER "${CROSS_TARGET_TRIPLET}gcc")

# CFLAGS
set (CMAKE_C_FLAGS "-g -Wextra -Wshadow -Wimplicit-function-declaration -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes -fno-common -ffunction-sections -fdata-sections -MD -Wall -Wundef -mthumb -mcpu=cortex-m0 " CACHE INTERNAL "c compiler flags")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -D USE_STDPERIPH_DRIVER  -D STM32F042  -D  INTERNAL_PULLUP  -D  USB_CLOCK_SOURCE_CRS ")

# CXXFLAGS
set (CMAKE_CXX_FLAGS "-Wextra -Wshadow -Wredundant-decls  -Weffc++ -fno-common -ffunction-sections -fdata-sections -MD -Wall -Wundef -mthumb -mcpu=cortex-m0 " CACHE INTERNAL "cxx compiler flags")

# ASMFLAGS
set (CMAKE_ASM_FLAGS "-g -mthumb -mcpu=cortex-m0 " CACHE INTERNAL "asm compiler flags")

# LDFLAGS
set (CMAKE_EXE_LINKER_FLAGS "-g -Wl,--gc-sections -Wl,-Map=NHC-Link042.map -mthumb -mcpu=cortex-m0 " CACHE INTERNAL "executable linker flags")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${CMAKE_SOURCE_DIR}/STM32FLASH.ld -L ")


# Generate the target
add_executable (${CMAKE_PROJECT_NAME}.elf  ${SRC_FILE0}  ${SRC_FILE1}  ${SRC_FILE2}  ${SRC_FILE3}  ${SRC_FILE4}  ${SRC_FILE5}  ${SRC_FILE6}  ${SRC_FILE7}  ${SRC_FILE8}  ${SRC_FILE9}  ${SRC_FILE10}  ${SRC_FILE11}  ${SRC_FILE12}  ${SRC_FILE13}  ${SRC_FILE14}  ${SRC_FILE15}  ${SRC_FILE16}  ${SRC_FILE17}  ${SRC_FILE18}  ${SRC_FILE19}  ${SRC_FILE20}  ${SRC_FILE21}  ${SRC_FILE22}  ${SRC_FILE23}  ${SRC_FILE24}  ${SRC_FILE25} )

# Link the library to the target
target_link_libraries (${CMAKE_PROJECT_NAME}.elf )

# Generate the binary file
add_custom_target (${CMAKE_PROJECT_NAME}.bin ALL arm-none-eabi-objcopy -Obinary "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.elf" "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.bin" DEPENDS ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.elf)

# Generate the hex file
add_custom_target (${CMAKE_PROJECT_NAME}.hex ALL arm-none-eabi-objcopy -Oihex "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.elf" "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.hex" DEPENDS ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.elf)

# Echo the size Infomation
add_custom_target (size ALL arm-none-eabi-size "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.elf" DEPENDS ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.elf)

# Make flash to the board by st-link
add_custom_target (flash COMMAND st-flash write ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.bin 0x8000000)

# Make clean-all
add_custom_target (clean-all COMMAND rm -rf ${CMAKE_BINARY_DIR}/*)