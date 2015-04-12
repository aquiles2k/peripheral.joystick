project(peripheral.joystick)

cmake_minimum_required(VERSION 2.6)
enable_language(CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

find_package(kodi REQUIRED)
find_package(kodiplatform REQUIRED)
find_package(Sdl)

include_directories(${INCLUDES}
                    ${kodiplatform_INCLUDE_DIRS}
                    ${SDL_INCLUDE_DIRS}
                    ${KODI_INCLUDE_DIR}
                    ${PROJECT_SOURCE_DIR}/src)

list(APPEND DEPLIBS ${kodiplatform_LIBRARIES})

set(JOYSTICK_SOURCES src/addon.cpp
                     src/api/Joystick.cpp
                     src/api/JoystickInterface.cpp
                     src/api/JoystickManager.cpp
                     src/devices/Buttons.cpp
                     src/devices/Device.cpp
                     src/devices/Devices.cpp
                     src/JoystickTranslator.cpp
                     src/log/Log.cpp
                     src/log/LogAddon.cpp
                     src/log/LogConsole.cpp
                     src/log/LogSyslog.cpp)

if(SDL_FOUND)
  add_definitions(-DHAVE_SDL)
  list(APPEND JOYSTICK_SOURCES src/api/sdl/JoystickInterfaceSDL.cpp
                               src/api/sdl/JoystickSDL.cpp)
  list(APPEND DEPLIBS ${SDL_LIBRARIES})
endif()

include(CheckIncludeFiles)
check_include_files(linux/joystick.h HAVE_LINUX_JOYSTICK_H)
if(HAVE_LINUX_JOYSTICK_H)
  add_definitions(-DHAVE_LINUX_JOYSTICK)
  list(APPEND JOYSTICK_SOURCES src/api/linux/JoystickInterfaceLinux.cpp
                               src/api/linux/JoystickLinux.cpp)
endif()

build_addon(peripheral.joystick JOYSTICK DEPLIBS)

include(CPack)
