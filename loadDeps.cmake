include(FetchContent)

file(MAKE_DIRECTORY deps)

FetchContent_Declare(libtiffGit
  GIT_REPOSITORY "https://gitlab.com/libtiff/libtiff.git"
  GIT_TAG "origin/cmake-test"   # it's much better to use a specific Git revision or Git tag for reproducibility
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/libtiff"
)
FetchContent_MakeAvailable(libtiffGit)

file(DOWNLOAD
        https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h
${CMAKE_CURRENT_SOURCE_DIR}/deps/stb_image_write.h)

file(DOWNLOAD
        https://raw.githubusercontent.com/ilia3101/MLV-App/master/src/mlv/liblj92/lj92.h
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/lj92.h)
file(DOWNLOAD
        https://raw.githubusercontent.com/ilia3101/MLV-App/master/src/mlv/liblj92/lj92.c
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/lj92.c)