cmake_minimum_required(VERSION 3.5)

IF(UNIX)
message("Running on Linux")

#Install VCPKG
execute_process(COMMAND ../vcpkg/bootstrap-vcpkg.sh -disableMetrics)
#Install the required libraries with vcpkg
execute_process(COMMAND ../vcpkg/vcpkg install glew glfw3 catch2 glm nlohmann-json freetype)
ELSE()
message("Running on Windows")
execute_process(COMMAND cmd /c "cd ${CMAKE_SOURCE_DIR} && .\\vcpkg\\bootstrap-vcpkg.bat -disableMetrics")
execute_process(COMMAND cmd /c "cd ${CMAKE_SOURCE_DIR} && .\\vcpkg\\vcpkg install glew glfw3 catch2 glm nlohmann-json freetype")
ENDIF()

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)
message(TOOLCHAIN FILE:)
message(${CMAKE_TOOLCHAIN_FILE})

project(main)

file(GLOB_RECURSE SRC
    "src/*.h"
    "src/*.cpp"
)
add_executable(main ${SRC})

#Decide to run in test mode
OPTION(RUN_TESTS "Run unit tests" OFF)
IF(RUN_TESTS MATCHES ON)
    message("====================================\nRunning With Unit Tests...\n====================================")
    ADD_DEFINITIONS(-DRUN_TESTS)
    find_package(Catch2 CONFIG REQUIRED)
    target_link_libraries(main PRIVATE Catch2::Catch2WithMain)
ENDIF()
unset(RUN_TESTS CACHE)

find_package(freetype CONFIG REQUIRED)
target_link_libraries(main PRIVATE freetype)

find_package(GLEW REQUIRED)
target_link_libraries(main PRIVATE GLEW::GLEW)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(main PRIVATE glfw)


find_package(glm CONFIG REQUIRED)
target_link_libraries(main PRIVATE glm::glm)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(main PRIVATE nlohmann_json::nlohmann_json)
