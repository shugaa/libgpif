INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib)

SET(testsrcs
    gpif_testcase.c)

ADD_EXECUTABLE(gpiftest ${testsrcs})
 
TARGET_LINK_LIBRARIES(gpiftest 
    gpif
    cunit)

INSTALL(TARGETS gpiftest DESTINATION bin)

