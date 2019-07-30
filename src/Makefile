
#变量
CC=gcc
INCLUDE=-I../include
CFLAGS=-c -g -Wall $(INCLUDE)
CPPFLAGS=-E-
DLDFLAGS=-lpthread -lssl -lcrypto -lmysqlclient
TARGET=app
RM=-rm -rf


#函数
SRCFILE=$(wildcard *.c)
DESFILE=$(patsubst %.c, %.o, $(SRCFILE))



$(TARGET):$(DESFILE)
	$(CC) -g $^ $(DLDFLAGS) -o $@

%.o:%.c
	$(CC) $(CFLAGS) -rdynamic $<

clean:
	$(RM) $(TARGET) $(DESFILE)
