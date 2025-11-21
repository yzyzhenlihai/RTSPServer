cmake_minimum_required(VERSION 3.10)
project(RtspServer)

# 添加编译选项
add_compile_options(-g)
# 引入头文件位置
include_directories(${PROJECT_SOURCE_DIR}/Live)
include_directories(${PROJECT_SOURCE_DIR}/Scheduler)
include_directories(${PROJECT_SOURCE_DIR}/Base)

# 引入源文件位置
aux_source_directory(${PROJECT_SOURCE_DIR}/Live LIVE_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/Scheduler SCHEDULER_SRC)

# 设置可执行文件位置
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_executable(rtspserver main.cpp ${LIVE_SRC} ${SCHEDULER_SRC})
