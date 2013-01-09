# -*- cmake -*-

include(OpenJPEG)

if (NOT USE_KDU)
 set(LLIMAGEJ2COJ_LIBRARIES llimagej2coj)
endif (NOT USE_KDU)