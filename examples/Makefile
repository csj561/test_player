# use pkg-config for getting CFLAGS and LDLIBS
CC=g++
FFMPEG_LIBS=    libavdevice                        \
                libavformat                        \
                libavfilter                        \
                libavcodec                         \
                libswresample                      \
                libswscale                         \
                libavutil                          \

CFLAGS += -Wall -g
CFLAGS := $(shell pkg-config --cflags $(FFMPEG_LIBS)) $(CFLAGS) 
LDLIBS := $(shell pkg-config --libs $(FFMPEG_LIBS)) $(LDLIBS) -lavtool -lsock
CPPFLAGS +=  -D__STDC_FORMAT_MACROS

EXAMPLES=       test1							   \

OBJS=$(addsuffix .o,$(EXAMPLES))

# the following examples make explicit use of the math library
decoding_encoding: LDLIBS += -lm
muxing:            LDLIBS += -lm
resampling_audio:  LDLIBS += -lm

.phony: all clean-test clean

all: $(OBJS) $(EXAMPLES)

clean-test:
	$(RM) test*.pgm test.h264 test.mp2 test.sw test.mpg *.o

clean: clean-test
	$(RM) $(EXAMPLES) $(OBJS)
