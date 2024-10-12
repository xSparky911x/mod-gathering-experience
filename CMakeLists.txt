cmake_minimum_required(VERSION 3.16)
project(GatheringExperienceModule)

include_directories(
    ${CMAKE_SOURCE_DIR}/src/server/shared
    ${CMAKE_SOURCE_DIR}/src/server/game
    ${CMAKE_SOURCE_DIR}/src/server/collision
)

add_library(GatheringExperienceModule SHARED
    GatheringExperienceModule.cpp
)

target_link_libraries(GatheringExperienceModule
    game
)