cmake_minimum_required(VERSION 3.5)

project(MathTools LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets OpenGL Charts CONFIG REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)

set(LINK_LIBRARIES
	Qt5::Widgets Qt5::Charts Qt5::OpenGL
)

set(INCLUDE_DIRS
    ${INCLUDE_DIRS}
    src
    ${EIGEN3_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
)

include_directories(${INCLUDE_DIRS})

add_executable(MathTools
    main.cpp
    src/common.h
    src/ui/CanvasView.h
    src/ui/CanvasView.cpp
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.ui
)

target_link_libraries(MathTools PRIVATE ${LINK_LIBRARIES})
