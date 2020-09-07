cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{DOLCESDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{DOLCESDK}/share/dolce.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define DOLCESDK to point to your SDK path!")
  endif()
endif()

project(Snake)

include("${DOLCESDK}/share/dolce.cmake" REQUIRED)

set(VITA_APP_NAME "Snake")
set(VITA_TITLEID  "JJHE00002")
set(VITA_VERSION  "01.00")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3 -fno-builtin")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS}")

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(${PROJECT_NAME}
  src/main.c
)

target_link_libraries(${PROJECT_NAME}
  vita2d_sys_stub
  vitaSAS_stub
  SceLibKernel_stub
  SceSysmem_stub
  SceKernelThreadMgr_stub
  SceProcessMgr_stub
  SceDisplay_stub
  SceDisplay_stub
  SceGxm_stub
  SceSysmodule_stub
  SceCtrl_stub
  ScePgf_stub
  ScePvf_stub
  SceCommonDialog_stub
  SceDbg_stub
  SceFios2_stub
  SceKernelDmacMgr_stub
  SceAppUtil_stub
  SceLibc_stub
  SceAppMgr_stub
  SceSas_stub
  SceAudio_stub
)

set_target_properties(${PROJECT_NAME}
  PROPERTIES LINK_FLAGS "-nostdlib"
)

# Budget: 16MB
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/boot_param.bin
  COMMAND dolce-make-bootparam app_memsize 0x4000 attribute 0x02 ${CMAKE_CURRENT_BINARY_DIR}/boot_param.bin
)

#set(DOLCE_ELF_CREATE_FLAGS "${DOLCE_ELF_CREATE_FLAGS} -h 1048576")
dolce_create_self(eboot.bin ${PROJECT_NAME} UNSAFE BOOT_PARAM ${CMAKE_CURRENT_BINARY_DIR}/boot_param.bin)

dolce_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} eboot.bin
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE sce_sys/icon0.png sce_sys/icon0.png
  FILE sce_sys/livearea/contents/bg0.png sce_sys/livearea/contents/bg0.png
  FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
  FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
  FILE src/resources/gamedata.psarc gamedata.psarc
  FILE src/resources/vita2d_sys.suprx module/vita2d_sys.suprx
  FILE src/resources/vitaSAS.suprx module/vitaSAS.suprx
)