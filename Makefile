LIBS = libdrm gbm egl glesv2 #dbus-1

CFLAGS += -g -std=gnu11 -Wall -Wextra -Wno-unused-parameter $(shell pkg-config --cflags $(LIBS)) -pthread
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
