# Which compiler
CC = gcc
AR	= ar
RANLIB     = ranlib

LIB_A	= libspider.a
# Options for development
CFLAGS = -g -Wall
# Options for release
# CFLAGS = -O -Wall

INCLUDE = -I../
#预处理选项-I -D等
CPPFLAGS = $(INCLUDE)
#make 搜索路径
VPATH = ../include/:../config_dir

# 需要的目标文件列表（.o文件)
OBJS =fixed_fifo.o fifo.o bf.o str.o wrapper.o dict.o zstream.o

all:$(OBJS)
	${AR} rv ${LIB_A} $?
	${RANLIB} ${LIB_A}
	-rm -f *.d *.d.*
clean:
	-rm -f *.o *.d *.d.* $(LIB_A)
.PHONY: clean

#自动处理头文件依赖
#SOURCES为所有的源文件列表
SOURCES = fixed_fifo.c fifo.c  bf.c str.c wrapper.c dict.c zstream.c
-include $(SOURCES:.c=.d)
%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$



