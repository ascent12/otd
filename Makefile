LIBS = libdrm gbm egl glesv2 libsystemd

CFLAGS += -g -std=c11 -Wall -Wextra -Wno-unused-parameter $(shell pkg-config --cflags $(LIBS)) -pthread
CPPFLAGS = -D_POSIX_C_SOURCE=200809L
LDFLAGS = -g -pthread
LDLIBS = $(shell pkg-config --libs $(LIBS))

EXE = main
SRC = $(wildcard *.c)
DEP = .depend

all: $(EXE)

%.o: %.c
	@mkdir -p $(DEP)/$(@D)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -MMD -MF $(DEP)/$*.d -o $@ $<

$(EXE): $(SRC:.c=.o)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	$(RM) -r $(EXE) $(SRC:.c=.o) $(DEP)

.PHONY: all clean

.PRECIOUS: $(DEP)/%.d

-include $(addprefix $(DEP)/, $(SRC:.c=.d))
