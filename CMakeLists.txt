cmake_minimum_required(VERSION 3.16)

set(PLAYDATE_GAME_NAME bullethell)
set(PLAYDATE_GAME_DEVICE ${PLAYDATE_GAME_NAME}_DEVICE)
project(${PLAYDATE_GAME_NAME} C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT DEFINED ENV{PLAYDATE_SDK_PATH})
  message(FATAL_ERROR "PLAYDATE_SDK_PATH is not set.")
endif()

set(SDK $ENV{PLAYDATE_SDK_PATH})

set(GAME_SOURCES
  src/main.c
  src/game.c
  src/entities.c
  src/spawn.c
  src/powerups.c
)

if (TOOLCHAIN STREQUAL "armgcc")
  add_executable(${PLAYDATE_GAME_DEVICE} ${GAME_SOURCES})
  set(GAME_TARGET ${PLAYDATE_GAME_DEVICE})
else()
  add_library(${PLAYDATE_GAME_NAME} SHARED ${GAME_SOURCES})
  set(GAME_TARGET ${PLAYDATE_GAME_NAME})
endif()

target_include_directories(${GAME_TARGET} PRIVATE
  src
  "${SDK}/C_API"
)

include("${SDK}/C_API/buildsupport/playdate_game.cmake")
