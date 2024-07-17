cmake_minimum_required(VERSION 3.16)

project(oel VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick SerialBus)

qt_standard_project_setup(REQUIRES 6.5)

add_subdirectory(../lib lib)

set(TARGET_NAME app${PROJECT_NAME})

qt_add_executable(${TARGET_NAME}
    main.cpp
)

qt_add_qml_module(${TARGET_NAME}
    URI oel
    VERSION 1.0
    QML_FILES
        BJM.qml
        CCVS.qml
        Dash.qml
        Debug1.qml
        Main.qml
        OEL.qml
        PDUList.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(${TARGET_NAME} PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appoel
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(${TARGET_NAME}
    PRIVATE Qt6::Quick Qt6::SerialBus j1939::qt
)

include(GNUInstallDirs)
install(TARGETS ${TARGET_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
