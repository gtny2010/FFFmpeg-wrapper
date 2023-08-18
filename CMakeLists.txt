cmake_minimum_required(VERSION 3.15)
project(TestDameon)

set(CMAKE_CXX_STANDARD 14)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

set(root_path /workspace/depends)

set(ffmpeg_path ${root_path}/libffmpeg5.0)
message(STATUS ${ffmpeg_path})

set(libx264_path ${root_path}/libx264)
message(STATUS ${libx264_path})

set(libx265_path ${root_path}/libx265)
message(STATUS ${libx265_path})

set(libxavs_path ${root_path}/libxavs)
message(STATUS ${libxavs_path})

set(libxavs2_path ${root_path}/libxavs2)
message(STATUS ${libxavs2_path})

set(libvpx_path ${root_path}/libvpx)
message(STATUS ${libvpx_path})

set(libspglog_path ${root_path}/libspdlog)
message(STATUS ${libspglog_path})

set(libgst_path ${root_path}/libspdlog)
message(STATUS ${libspglog_path})


# set(libopencv_path ${root_path}/libopencv-n4.6)
# message(STATUS ${libspglog_path})

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/workspace/depends/libopencv-n4.6/lib/cmake/opencv4")
find_package(OpenCV REQUIRED )#core videoio
message( STATUS "OpenCV_INCLUDE_DIRS=${OpenCV_INCLUDE_DIRS}" )
message( STATUS "OpenCV_LIBS=${OpenCV_LIBS}" )

# include dirs
include_directories(${ffmpeg_path}/include ${libspglog_path}/include ${OpenCV_INCLUDE_DIRS})

link_directories(${libspglog}/lib ${libx265_path}/lib ${libxavs2_path}/lib ${libxavs_path}/lib ${libvpx_path}/lib)
link_directories(${ffmpeg_path}/lib ${libx264_path}/lib "/workspace/depends/libopencv-n4.6/lib")



get_property( dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES )

foreach( dir ${dirs} )
    message( STATUS "include dir=${dir}" )
endforeach()


set(src_list main.cpp
        InstanceName.cpp
        ThreadRunner.cpp
        SafeQueue.cpp
        FpsGauge.cpp
        JobPkgBase.cpp
        Survey.cpp
        DemuxerWorker.cpp
        MuxWorker.cpp
        Factory.cpp
        YuvSurvey.cpp
        H264Survey.cpp
        Allsurvey.cpp 
        BmpSurvey.cpp
        )

aux_source_directory(${PROJECT_SOURCE_DIR}/xlog xlog_list)
aux_source_directory(${PROJECT_SOURCE_DIR}/videoio videoio_list)

add_executable(TestDameon ${src_list} ${xlog_list} ${videoio_list})

target_link_libraries(TestDameon pthread avformat avcodec avutil swscale swresample dl z ${OpenCV_LIBS})
target_include_directories(TestDameon SYSTEM PUBLIC ${libspdlog}/include)

