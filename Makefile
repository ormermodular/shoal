# Shoal - 8-track generative melody sequencer for the Expert Sleepers disting NT
#
# Needs the GNU Arm Embedded Toolchain (arm-none-eabi-c++) and the
# distingNT_API headers. `make api` fetches the API, pinned to the commit
# Shoal is written against; then `make`.

NT_API_PATH ?= distingNT_API
# API v13 (firmware 1.15.0+). A newer SDK is refused by a static_assert in
# src/shoal.cpp rather than silently raising the firmware requirement.
NT_API_REV  ?= 9aeda6d

CXX      := arm-none-eabi-c++
CXXFLAGS := -std=c++11 -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb \
            -fno-rtti -fno-exceptions -Os -fPIC -Wall -I$(NT_API_PATH)/include

HOSTCXX  := g++ -std=gnu++11 -O1 -w -I$(NT_API_PATH)/include -DSHOAL_CPP='"../src/shoal.cpp"'
HOSTLINK := -Wl,--unresolved-symbols=ignore-all

# Undefined symbols the NT loader resolves for a plug-in: its own API, the
# GOT, and the few libc/libm functions the firmware exports.
ALLOWED_SYMS := ^(NT_[A-Za-z0-9_]+|_GLOBAL_OFFSET_TABLE_|memcpy|memmove|memset|cosf|expf|powf|sinf)$$

all: plugins/shoal.o

plugins/shoal.o: src/shoal.cpp $(NT_API_PATH)/include/distingnt/api.h
	mkdir -p plugins
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(NT_API_PATH)/include/distingnt/api.h: api

api:
	@test -d $(NT_API_PATH) || { git clone https://github.com/expertsleepersltd/distingNT_API.git $(NT_API_PATH) \
		&& git -C $(NT_API_PATH) checkout --quiet $(NT_API_REV); }

# Quick host-compiler syntax check (no ARM toolchain needed)
check:
	g++ -std=gnu++11 -fsyntax-only -Wall -I$(NT_API_PATH)/include src/shoal.cpp

# Host tests (no hardware): parameter table and pages, clocking (per-track
# clocks, master-clock regression hashes), and construct() with junk-filled
# memory. See the header of each file in test/.
test: check-params check-clocks check-init

check-params:
	$(HOSTCXX) test/params.cpp -o /tmp/shoal-params $(HOSTLINK)
	/tmp/shoal-params

check-clocks:
	$(HOSTCXX) test/clock_tests.cpp -o /tmp/shoal-clock-tests $(HOSTLINK)
	/tmp/shoal-clock-tests | grep -v REGRESS

# The NT does not hand out zeroed memory: junk-fill both blocks, run silent
# input, and require output bit-identical to zero-filled memory.
check-init:
	$(HOSTCXX) test/init_junk.cpp -o /tmp/shoal-init-junk $(HOSTLINK)
	/tmp/shoal-init-junk

# Release checks on the built object: version stamp, undefined symbols
# limited to what the firmware exports, no writable static data.
verify: plugins/shoal.o
	@echo "stamp:   $$(strings plugins/shoal.o | grep -E '^v[0-9]+\.[0-9]+\.[0-9]+' )"
	@bad=$$(arm-none-eabi-nm -u plugins/shoal.o | awk '{print $$2}' | grep -Ev '$(ALLOWED_SYMS)'); \
		test -z "$$bad" || { echo "unresolved symbols the NT does not export:"; echo "$$bad"; exit 1; }
	@echo "symbols: ok ($$(arm-none-eabi-nm -u plugins/shoal.o | wc -l) undefined, all exported by the firmware)"
	@arm-none-eabi-size -A plugins/shoal.o | awk '/^\.(data|bss|sdata|sbss) / && $$2 > 0 { print "writable static data: " $$0; bad=1 } END { exit bad }'
	@echo "data:    no writable statics"

# Release archive: no version in the name (the Gallery follows the latest
# release), the NT's own folder layout inside.
release: verify
	rm -rf build/zip shoal-plugin.zip
	mkdir -p build/zip/programs/plug-ins/ormermodular
	cp plugins/shoal.o build/zip/programs/plug-ins/ormermodular/shoal.o
	cd build/zip && zip -q -r ../../shoal-plugin.zip programs
	@unzip -l shoal-plugin.zip

clean:
	rm -rf plugins build shoal-plugin.zip

.PHONY: all api check test check-params check-clocks check-init verify release clean
