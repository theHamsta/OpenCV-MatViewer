
cmake_minimum_required(VERSION 2.6)
project(opencvmatviewer)


set(QT_MIN_VERSION "5.3.0")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package( OpenCV REQUIRED )
find_package( Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui Widgets )
find_package( OpenGL 2.0 REQUIRED )

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
    )
    

aux_source_directory( source SOURCES )

file(GLOB UI RELATIVE  "${CMAKE_CURRENT_SOURCE_DIR}" ${CMAKE_CURRENT_SOURCE_DIR}/ui/*.ui )
file(GLOB UI_INCLUDES RELATIVE  "${CMAKE_CURRENT_SOURCE_DIR}" ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp )
qt5_wrap_cpp(MOC_SRC ${UI_INCLUDES})
qt5_wrap_ui(MOC_UI ${UI})

add_library( opencvmatviewer STATIC
		${SOURCES}
		${MOC_SRC}
		${UI}
		${UI_INCLUDES}
		)
target_link_libraries(opencvmatviewer ${Qt6Widgets_LIBRARIES} ${OpenCV_LIBS} ${OPENGL_LIBRARIES})
install ( TARGETS opencvmatviewer LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


