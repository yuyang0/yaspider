SET (LIBSPIDER_SRC fixed_fifo.c fifo.c
                   bf.c str.c wrapper.c
                   dict.c zstream.c)

ADD_LIBRARY(spider_static STATIC ${LIBSPIDER_SRC})
SET_TARGET_PROPERTIES(spider_static PROPERTIES OUTPUT_NAME "spider")
SET_TARGET_PROPERTIES(spider_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

ADD_LIBRARY(spider_shared SHARED ${LIBSPIDER_SRC})
SET_TARGET_PROPERTIES(spider_shared PROPERTIES OUTPUT_NAME "spider")
SET_TARGET_PROPERTIES(spider_shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)
