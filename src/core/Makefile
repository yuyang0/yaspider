#对本makefile进行修改定制时要修改的地方如下：
# EXE_FILE 修改成实际的可执行文件名
# INCLUDE 修改成自定义的头文件目录
# VPATH 修改成需要make搜索的目录
# INSTDIR 修改成实际的安装目录
# OBJECTS 修改成EXE_FILE依赖的目标文件
# SOURCES 修改成工程实际包含的所有源文件

#最终生成的可执行文件名
EXE_FILE = spider

all:$(EXE_FILE)

# Which compiler
CC = gcc

# Options for development
CFLAGS = -g -Wall -std=gnu99 -D_GNU_SOURCE
# Options for release
# CFLAGS = -O -Wall –std=c99

#for BerkeleyDB
INCLUDE= -I../lib -I../ -I../.deps/zlog/src
LIB_PATH = -L../lib -L./ -L../.deps/zlog/src
#预处理选项-I -D等
CPPFLAGS = $(INCLUDE)
#make 搜索路径
VPATH = ../lib:/config_dir
#安装路径
INSTDIR = ~/tmp
# 需要的目标文件列表（.o文件)
OBJS = conf.o connection.o \
       http.o init.o init_ev.o log.o   \
       main.o parse.o site.o siteht.o save.o server.o\
       url.o url_bf.o url_fifo.o

$(EXE_FILE):$(OBJS)
	$(CC) -rdynamic $(LIB_PATH) -o $@ $^ -lspider -levent -lm -ldb -lzlog -lz -pthread
	-rm *.d *.d.*

test:test_conns
#connection test
test_conns: connection.o conf.o url.o site.o http.o log.o
	gcc -g -Wall -o test_conn connection.c conf.o url.o site.o http.o log.o -I../lib -L../lib -lspider -lz -lzlog -pthread
test_conf: conf.o
	gcc -g -Wall -I../lib -L../lib -o test_conf conf.c -lspider
# init_ev.o: init_ev.c
# 	$(CC) $(INCLUDE) -std=gnu99 -g -Wall -c init_ev.c
# url_fifo.o: url_fifo.c
# 	$(CC) $(INCLUDE) -g -Wall -c url_fifo.c
clean:
	-rm -f *.o $(EXE_FILE) *.d *.db core
.PHONY: clean
#自动处理头文件依赖
#SOURCES为所有的源文件列表
SOURCES = conf.c connection.c \
          http.c init.c init_ev.c log.c  \
          main.c parse.c site.c siteht.c save.c server.c\
          url.c url_bf.c url_fifo.c
# ignore the warn message "XXX.d: No such file or directory"
-include $(SOURCES:.c=.d)
%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

install: all
	@if [ -d $(INSTDIR) ]; \
	then \
	cp myapp $(INSTDIR);\
	chmod a+x $(INSTDIR)/$(EXE_FILE);\
	chmod og-w $(INSTDIR)/$(EXE_FILE);\
	echo “Installed in $(INSTDIR)“;\
	else \
	echo “Sorry, $(INSTDIR) does not exist”;\
	fi

