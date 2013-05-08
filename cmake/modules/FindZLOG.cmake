#
# configuration to find ZLOG library
#

# Include dir
find_path(ZLOG_INCLUDE_DIR
  NAMES zlog.h
  PATHS /usr/include/ /usr/local/include/ ${PROJECT_SOURCE_DIR}/.deps/zlog/src
)

# Finally the library itself
find_library(ZLOG_LIBRARY
  NAMES  zlog
  PATHS /usr/lib/ /usr/local/lib/  ${PROJECT_SOURCE_DIR}/.deps/zlog/src
)

IF (ZLOG_INCLUDE_DIR AND ZLOG_LIBRARY)
	SET(ZLOG_FOUND TRUE)
ENDIF (ZLOG_INCLUDE_DIR AND ZLOG_LIBRARY)

IF (ZLOG_FOUND)
	IF (NOT ZLOG_FIND_QUIETLY)
		MESSAGE(STATUS "Found ZLOG: ${ZLOG_LIBRARY}")
	ENDIF (NOT ZLOG_FIND_QUIETLY)
ELSE (ZLOG_FOUND)
		IF (ZLOG_FIND_REQUIRED)
			MESSAGE(FATAL_ERROR "Could not find zlog library")
		ENDIF (ZLOG_FIND_REQUIRED)
ENDIF (ZLOG_FOUND)
