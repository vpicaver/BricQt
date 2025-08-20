cmake_minimum_required(VERSION 3.16)

project(BricQt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Bluetooth)

qt_standard_project_setup(REQUIRES 6.8)

# Generate Info.plist from template
configure_file(${CMAKE_SOURCE_DIR}/Info.plist.in
               ${CMAKE_BINARY_DIR}/Info.plist @ONLY)

qt_add_executable(appBricQt
    main.cpp
    BluetoothPermission.mm
)

qt_add_qml_module(appBricQt
    URI BricQt
    VERSION 1.0
    QML_FILES
        Main.qml
        SOURCES BricClient.h BricClient.cpp
        SOURCES BricMeasurementModel.h BricMeasurementModel.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appBricQt PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.vadosesolutions.appBricQt
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_BINARY_DIR}/Info.plist"
)

target_link_libraries(appBricQt
    PRIVATE Qt6::Quick
    PRIVATE Qt6::Bluetooth
)
target_link_libraries(appBricQt PRIVATE Qt6::Core)

include(GNUInstallDirs)
install(TARGETS appBricQt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


