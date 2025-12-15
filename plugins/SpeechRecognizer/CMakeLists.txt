set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set(PLUGIN "SpeechRecognizer")

set(
    SRC
    plugin.cpp
    speech_recognizer.cpp
)

set(CMAKE_AUTOMOC ON)

# Find required Qt packages
find_package(Qt5 REQUIRED COMPONENTS Core Qml Quick Multimedia)

# Determine architecture-specific library path
execute_process(
    COMMAND dpkg-architecture -qDEB_HOST_ARCH
    OUTPUT_VARIABLE TARGET_ARCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(TARGET_ARCH STREQUAL "arm64" OR TARGET_ARCH STREQUAL "aarch64")
    set(VOSK_LIB_DIR "${CMAKE_SOURCE_DIR}/libs/vosk-arm64")
elseif(TARGET_ARCH STREQUAL "armhf")
    set(VOSK_LIB_DIR "${CMAKE_SOURCE_DIR}/libs/vosk-armhf")
else()
    set(VOSK_LIB_DIR "${CMAKE_SOURCE_DIR}/libs/vosk-amd64")
endif()

message(STATUS "Using Vosk library from: ${VOSK_LIB_DIR}")

# Add Vosk include path
include_directories(${VOSK_LIB_DIR})

add_library(${PLUGIN} MODULE ${SRC})
set_target_properties(${PLUGIN} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PLUGIN})

# Link Qt modules
target_link_libraries(${PLUGIN} 
    Qt5::Core 
    Qt5::Qml 
    Qt5::Quick 
    Qt5::Multimedia
)

# Link Vosk library
set(VOSK_LIB_PATH "${VOSK_LIB_DIR}/libvosk.so")
if(EXISTS ${VOSK_LIB_PATH})
    target_link_libraries(${PLUGIN} ${VOSK_LIB_PATH})
    message(STATUS "Found libvosk.so at ${VOSK_LIB_PATH}")
else()
    message(FATAL_ERROR "libvosk.so not found at ${VOSK_LIB_PATH}. Run ./setup.sh <arch> first.")
endif()

# Set RPATH to find libvosk.so at runtime
set_target_properties(${PLUGIN} PROPERTIES
    INSTALL_RPATH "$ORIGIN/../../libs/vosk:$ORIGIN/../../../libs/vosk"
    BUILD_WITH_INSTALL_RPATH TRUE
)

execute_process(
    COMMAND dpkg-architecture -qDEB_HOST_MULTIARCH
    OUTPUT_VARIABLE ARCH_TRIPLET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(QT_IMPORTS_DIR "/lib/${ARCH_TRIPLET}")

install(TARGETS ${PLUGIN} DESTINATION ${QT_IMPORTS_DIR}/${PLUGIN}/)
install(FILES qmldir DESTINATION ${QT_IMPORTS_DIR}/${PLUGIN}/)
