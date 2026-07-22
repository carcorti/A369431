CC ?= gcc
WARN := -Wall -Wextra -Wconversion -Wshadow -Wstrict-prototypes -Wmissing-prototypes -Wformat=2 -Wundef
# __int128 is deliberately required by the numerical contract; GCC and Clang
# support it in C17 mode, so ISO-pedantic is intentionally not enabled.
OUTPUT_DIRECTORY := $(abspath ../results)
CPPFLAGS := -DSOURCE_PATH=\"$(abspath a369431.c)\" -DOUTPUT_DIRECTORY=\"$(OUTPUT_DIRECTORY)\"
# The initial campaign cap is a fixed source-level contract (24), not a
# make-time tuning parameter. CPPFLAGS remain active for diagnostic builds.
CFLAGS ?= -O3 -std=c17 $(WARN)
LDFLAGS ?=
BIN := a369431

.PHONY: all test sanitize clang static clean

all: $(BIN) | $(OUTPUT_DIRECTORY)

$(OUTPUT_DIRECTORY):
	mkdir -p "$@"

$(BIN): a369431.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@ $(LDFLAGS)

test: $(BIN)
	./$(BIN) selftest

sanitize:
	$(CC) $(CPPFLAGS) -std=c17 -O1 -g $(WARN) -fsanitize=address,undefined -fno-omit-frame-pointer a369431.c -o $(BIN)_san
	# LeakSanitizer cannot run under this managed ptrace environment; ASan/UBSan remain enabled.
	ASAN_OPTIONS=detect_leaks=0 ./$(BIN)_san selftest

clang:
	clang $(CPPFLAGS) -O3 -std=c17 $(WARN) a369431.c -o $(BIN)_clang
	./$(BIN)_clang selftest

static:
	clang-tidy -checks='-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling' a369431.c -- $(CPPFLAGS) -std=c17 $(WARN)
	cppcheck --enable=warning,style,performance,portability --std=c11 --quiet a369431.c

clean:
	rm -f $(BIN) $(BIN)_san $(BIN)_clang
