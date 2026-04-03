cmake_minimum_required(VERSION 3.16)

project(image_processor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)

find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBHEIF REQUIRED libheif)

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui)

add_executable(image_processor
        src/main.cpp
        src/parser.cpp
        src/processor.cpp
        src/bmp.cpp
        src/png_codec.cpp
        src/jpeg_codec.cpp
        src/heic_codec.cpp
        src/image_io.cpp
        src/image.cpp
        src/pipeline.cpp
        src/filters.cpp
        src/filter_factory.cpp
        src/pipeline_builder.cpp
)

target_include_directories(image_processor PRIVATE
        include
        ${LIBHEIF_INCLUDE_DIRS}
)

target_link_directories(image_processor PRIVATE
        ${LIBHEIF_LIBRARY_DIRS}
)

target_compile_options(image_processor PRIVATE
        ${LIBHEIF_CFLAGS_OTHER}
)

target_link_options(image_processor PRIVATE
        ${LIBHEIF_LDFLAGS_OTHER}
)

target_link_libraries(image_processor PRIVATE
        PNG::PNG
        JPEG::JPEG
        ${LIBHEIF_LIBRARIES}
)

add_executable(image_processor_gui
        src/gui_main.cpp
        src/main_window.cpp
        src/image_qt_converter.cpp
        src/drop_image_label.cpp

        include/main_window.h
        include/image_qt_converter.h
        include/drop_image_label.h

        src/parser.cpp
        src/processor.cpp
        src/bmp.cpp
        src/png_codec.cpp
        src/jpeg_codec.cpp
        src/heic_codec.cpp
        src/image_io.cpp
        src/image.cpp
        src/pipeline.cpp
        src/filters.cpp
        src/filter_factory.cpp
        src/pipeline_builder.cpp
)

target_include_directories(image_processor_gui PRIVATE
        include
        ${LIBHEIF_INCLUDE_DIRS}
)

target_link_directories(image_processor_gui PRIVATE
        ${LIBHEIF_LIBRARY_DIRS}
)

target_compile_options(image_processor_gui PRIVATE
        ${LIBHEIF_CFLAGS_OTHER}
)

target_link_options(image_processor_gui PRIVATE
        ${LIBHEIF_LDFLAGS_OTHER}
)

target_link_libraries(image_processor_gui PRIVATE
        Qt6::Widgets
        Qt6::Gui
        PNG::PNG
        JPEG::JPEG
        ${LIBHEIF_LIBRARIES}
)

add_executable(unit_tests
        tests.cpp
        src/parser.cpp
        src/bmp.cpp
        src/image.cpp
        src/pipeline.cpp
        src/filter_factory.cpp
        src/pipeline_builder.cpp
        src/filters.cpp
)

target_include_directories(unit_tests PRIVATE
        include
)

if (MSVC)
    target_compile_options(image_processor PRIVATE /W4)
    target_compile_options(image_processor_gui PRIVATE /W4)
    target_compile_options(unit_tests PRIVATE /W4)
else()
    target_compile_options(image_processor PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(image_processor_gui PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(unit_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()