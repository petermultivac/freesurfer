project(mri_mcsim)

include_directories(${FS_INCLUDE_DIRS})

add_executable(mri_mcsim mri_mcsim.cpp)
target_link_libraries(mri_mcsim utils)

install(TARGETS mri_mcsim DESTINATION bin)
