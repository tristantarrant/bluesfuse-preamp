# BB Preamp - Xotic BB Preamp emulation (1.5 / AT / Bass / Comp / MB)
#
# Targets:
#   make          build the .so into the LV2 bundle
#   make tf       regenerate src/bbpreamp_tf.lib from the circuit (sympy)
#   make dsp      regenerate the faust-generated C++ from the .dsp source
#   make install  copy the bundle to $(INSTALL_DIR) (default: /usr/lib/lv2)
#   make test     build and run the LV2 test harness
#   make clean    remove build products

PLUG    := bbpreamp
BUNDLE  := $(PLUG).lv2
DSP     := src/$(PLUG).dsp
TF      := src/$(PLUG)_tf.lib
CC_GEN  := src/$(PLUG).cc
HDR     := src/$(PLUG).h
CPP     := src/$(PLUG).cpp
SO      := $(BUNDLE)/$(PLUG).so

INSTALL_DIR ?= /usr/lib/lv2

CXX     ?= g++
# -ffast-math: ~35% less CPU, identical response (no NaN/Inf logic in the DSP)
CXXFLAGS := -O3 -ffast-math -fPIC -fvisibility=hidden -Wall -Wextra -Wno-unused-parameter
LDFLAGS  := -Wl,--exclude-libs,ALL

# Path to the guitarix dsp2cc tool (used by the 'dsp' target only;
# the checked-in generated C++ builds without it).
DSP2CC  ?= /home/tst/Fun/guitarix/trunk/tools/dsp2cc

.PHONY: all tf dsp so install test clean

all: so

# ---- regenerate the linear-stage transfer functions (needs sympy) ------
tf:
	python3 tools/gen_tf.py > $(TF).tmp && mv $(TF).tmp $(TF)

# ---- regenerate the dsp class from the faust source -------------------
dsp:
	python3 $(DSP2CC) --init-type=plugin-lv2 -double -o $(CC_GEN) $(DSP)

# ---- compile ------------------------------------------------------------
so: $(SO)

$(SO): $(CPP) $(CC_GEN) $(HDR) src/bb_glue.h
	$(CXX) $(CXXFLAGS) -shared -o $@ $(CPP) $(LDFLAGS)

install: so
	mkdir -p $(INSTALL_DIR)
	rm -rf "$(INSTALL_DIR)/$(BUNDLE)"
	cp -r $(BUNDLE) "$(INSTALL_DIR)/"

test: so
	$(CC) -O2 -o bb_test tests/bb_test.c -ldl -lm
	./bb_test $(CURDIR)/$(SO)

clean:
	rm -f $(SO) bb_test
