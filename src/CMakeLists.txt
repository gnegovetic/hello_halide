cmake_minimum_required(VERSION 3.13)
project(ContrastAdjust)

#find_package(Halide REQUIRED)
set(HELIDE_SUPPORT_PATH /opt/homebrew/Cellar/halide/13.0.4)
set(GENERATED_CONTRAST_ADJUST contrast_adjust)
set(GENERATED_MINMAX min_max)

set(CMAKE_BUILD_TYPE Release)
add_compile_options(-std=c++17 -fno-rtti)

#### Generator
add_executable(contrastAdjustGenerator 
    ContrastAdjustGenerator.cpp
    ${HELIDE_SUPPORT_PATH}/share/tools/GenGen.cpp
)

target_include_directories(contrastAdjustGenerator PRIVATE /opt/homebrew/include/)
target_link_directories(contrastAdjustGenerator PRIVATE /opt/homebrew/lib/)
target_link_libraries(contrastAdjustGenerator Halide)

#### Run generators with Adams2019 auto-scheduler
add_custom_command(TARGET contrastAdjustGenerator POST_BUILD 
    COMMAND ./contrastAdjustGenerator -g ${GENERATED_MINMAX} -e c_header,static_library,stmt_html,schedule 
            -p ${HELIDE_SUPPORT_PATH}/lib/libautoschedule_adams2019.so -s Adams2019 -o . target=host auto_schedule=true
    BYPRODUCTS ${GENERATED_MINMAX}.h ${GENERATED_MINMAX}.a ${GENERATED_MINMAX}.stmt.html ${GENERATED_MINMAX}.schedule.h
    COMMENT "Generating min max library")

add_custom_command(TARGET contrastAdjustGenerator POST_BUILD 
    COMMAND ./contrastAdjustGenerator -g ${GENERATED_CONTRAST_ADJUST} -e c_header,static_library,stmt_html,schedule 
            -p ${HELIDE_SUPPORT_PATH}/lib/libautoschedule_adams2019.so -s Adams2019  -o . target=host auto_schedule=true
    BYPRODUCTS ${GENERATED_CONTRAST_ADJUST}.h ${GENERATED_CONTRAST_ADJUST}.a ${GENERATED_CONTRAST_ADJUST}.stmt.html ${GENERATED_CONTRAST_ADJUST}.schedule.h
    COMMENT "Generating contrast adjust library")


#### Test app
add_executable(contrastAdjustTest
    ContrastAdjust.cpp 
)

target_include_directories(contrastAdjustTest PRIVATE ${CMAKE_BINARY_DIR} /opt/homebrew/include/ ${HELIDE_SUPPORT_PATH}/share/tools)
target_link_libraries(contrastAdjustTest ${CMAKE_BINARY_DIR}/${GENERATED_MINMAX}.a ${CMAKE_BINARY_DIR}/${GENERATED_CONTRAST_ADJUST}.a)
add_dependencies(contrastAdjustTest contrastAdjustGenerator)
