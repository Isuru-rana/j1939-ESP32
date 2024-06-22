cmake_minimum_required(VERSION 3.16)

project(j1939-lib VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QT_QML_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR})

find_package(Qt6 6.2 COMPONENTS Quick SerialBus REQUIRED)

include(FetchContent)

add_subdirectory(../../../src/j1939 j1939)

set(INCLUDES
    ca.h
    ca/ccvs.h
    ca/lighting_command.h
    ca/oel.h
    cs/base.h
    cs/generic.h
    cs/generic.hpp
    cs/network.h
    cs/tp.h
    data_field.h
    fwd.h
    NAME.h
    pdu.h
    plugin.h
    session.h
    transport.h
)

list(TRANSFORM INCLUDES PREPEND  include/j1939/qt/)

set(PLUGIN_NAME ${PROJECT_NAME}plugin)

qt_add_library(${PROJECT_NAME} STATIC)
qt_add_qml_module(${PROJECT_NAME}
    URI j1939.ui
    VERSION 1.0
    QML_FILES
        CAContainer.qml
        J1939LibControls.qml
        PDU.qml
    SOURCES
        ${INCLUDES}
        ca/ccvs.cpp
        ca/lighting_command.cpp
        ca/oel.cpp
        cs/generic.cpp
        cs/network.cpp
        cs/tp.cpp
        plugin.cpp
        session.cpp
        j1939lib.cpp j1939lib.h
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# We want fold expressions
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Quick Qt6::SerialBus)
target_link_libraries(${PROJECT_NAME}
    PUBLIC Qt6::Core embr::can embr::j1939)

add_library(j1939::qt ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PLUGIN_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

