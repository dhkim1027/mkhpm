SRCS = main.c md5.c hexfile.c
OBJS = $(SRCS:.c=.o)

TARGET = mkhpm

CFLAGS += -W -Wall

all : $(TARGET)
$(TARGET) : $(OBJS) $(LIBS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

%.o : %.c
	@echo compiling... $<
	$(CC) $(CFLAGS) $(CPPFLAGS) -c  $<

clean :
	rm -rf $(OBJS) $(TARGET)

