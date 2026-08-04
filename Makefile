# Shoal — 8-track generative melody sequencer for the Expert Sleepers disting NT
#
# Requires the GNU Arm Embedded Toolchain (arm-none-eabi-c++) and the
# distingNT_API headers. Run `make api` to fetch the API, then `make`.

NT_API_PATH ?= distingNT_API

CXX      := arm-none-eabi-c++
CXXFLAGS := -std=c++11 -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb \
            -fno-rtti -fno-exceptions -Os -fPIC -Wall -I$(NT_API_PATH)/include

all: plugins/shoal.o

plugins/shoal.o: src/shoal.cpp $(NT_API_PATH)/include/distingnt/api.h
	mkdir -p plugins
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(NT_API_PATH)/include/distingnt/api.h: api

api:
	@test -d $(NT_API_PATH) || git clone https://github.com/expertsleepersltd/distingNT_API.git $(NT_API_PATH)

# Quick host-compiler syntax check (no ARM toolchain needed)
check:
	g++ -std=gnu++11 -fsyntax-only -Wall -I$(NT_API_PATH)/include src/shoal.cpp

clean:
	rm -rf plugins

.PHONY: all api check clean
