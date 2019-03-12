cmake_minimum_required(VERSION 2.8)

project(bluetooth_tools)
include_directories(/usr/include)
include_directories(/usr/include/dbus-1.0)
include_directories(/usr/lib/x86_64-linux-gnu/dbus-1.0/include)
include_directories(/usr/include/glib-2.0)
include_directories(/usr/lib/x86_64-linux-gnu/glib-2.0/include)
add_executable(${PROJECT_NAME} "main.cpp" "bluetoothtools.h" "bluetoothtools.cpp")

find_library(GLIB_LIBRARY glib-2.0 HINTS /usr/lib/x86_64-linux-gnu)
find_library(GIO_LIBRARY gio-2.0 HINTS /usr/lib/x86_64-linux-gnu)
find_library(GOBJECT_LIBRARY gobject-2.0 HINTS /usr/lib/x86_64-linux-gnu)
find_library(DBUS_LIBRARY dbus-1 HINTS /usr/lib/x86_64-linux-gnu)
find_library(DBUS_GLIB_LIBRARY dbus-glib-1 HINTS /usr/lib/x86_64-linux-gnu)

target_link_libraries(${PROJECT_NAME} PUBLIC ${DBUS_LIBRARY})
target_link_libraries(${PROJECT_NAME} PUBLIC ${DBUS_GLIB_LIBRARY})
target_link_libraries(${PROJECT_NAME} PUBLIC ${GLIB_LIBRARY})
target_link_libraries(${PROJECT_NAME} PUBLIC ${GIO_LIBRARY})
target_link_libraries(${PROJECT_NAME} PUBLIC ${GOBJECT_LIBRARY})


link_directories(/usr/lib/x86_64-linux-gnu)
target_link_libraries(${PROJECT_NAME} bluetooth)
