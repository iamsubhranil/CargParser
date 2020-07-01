override CFLAGS += -Wall -Wextra
override LDFLAGS +=

RM=rm -f
# $(wildcard *.cpp /xxx/xxx/*.cpp): get all .cpp files from the current directory and dir "/xxx/xxx/"
SRCS := $(wildcard *.c)
# $(patsubst %.cpp,%.o,$(SRCS)): substitute all ".cpp" file name strings to ".o" file name strings
OBJS := $(patsubst %.c,%.o,$(SRCS))

# Allows one to enable verbose builds with VERBOSE=1
V := @
ifeq ($(VERBOSE),1)
	V :=
endif

all: release

debug: CFLAGS += -O0 -g3
debug: lib driver
	$(CC) $(LDFLAGS) -o cargparser $(OBJS)

release: CFLAGS += -O3
release: LDFLAGS += -s
release: lib driver
	$(CC) $(LDFLAGS) -o cargparser $(OBJS)

lib_release: CFLAGS += -O3
lib_release: LDFLAGS += -s
lib_release: lib

lib: cargparser.o
driver: main.o

depend: .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CC) $(CFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) *~ .depend

include .depend

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
