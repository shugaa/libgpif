SET(libsrcs
    gpif.c)
 
ADD_LIBRARY(gpif SHARED ${libsrcs})

INSTALL(TARGETS gpif 
    DESTINATION lib)

INSTALL(DIRECTORY . 
    DESTINATION include/gpif
    FILES_MATCHING REGEX "gpif.h")

