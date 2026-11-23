# BluesFuse Preamp - Xotic BB Preamp emulation (1.5 / AT / Bass / Comp / MB)
#
# Built with DPF: one codebase -> LV2 (+ modgui), VST3, CLAP and a JACK
# standalone app. The DSP is the faust-generated class in src/.
#
# Targets:
#   make               build all formats into bin/
#   make NOUI=true     DSP only (headless build hosts, e.g. Stom-Pi)
#   make install       install the LV2 bundle into $(INSTALL_DIR)
#   make install-all   also VST3, CLAP and the JACK app
#   make test          LV2 C-API test harness (DSP behaviour)
#   make test-ui       drive the real UI under Xvfb (needs xvfb-run, XTest)
#   make test-modgui   check the modgui in headless Chrome (needs PIL)
#   make test-clap     CLAP load/process smoke test
#   make test-all      all of the above
#   make tf            regenerate src/bluesfuse_tf.lib from the circuit (sympy)
#   make dsp           regenerate src/bluesfuse.cc from the .dsp
#   make modgui        re-render the modgui from the artwork
#   make clean

PLUG    := bluesfuse
BUNDLE  := bin/$(PLUG).lv2
DSP     := src/$(PLUG).dsp
TF      := src/$(PLUG)_tf.lib
CC_GEN  := src/$(PLUG).cc

PREFIX      ?= /usr
INSTALL_DIR ?= $(PREFIX)/lib/lv2
VST3_DIR    ?= $(PREFIX)/lib/vst3
CLAP_DIR    ?= $(PREFIX)/lib/clap
BIN_DIR     ?= $(PREFIX)/bin

# Path to the guitarix dsp2cc tool (used by 'make dsp' only; the
# checked-in generated C++ builds without it).
DSP2CC  ?= /home/tst/Fun/guitarix/trunk/tools/dsp2cc

TTL_GEN := dpf/utils/lv2_ttl_generator

.PHONY: all plugin ttl tf dsp modgui install install-all test test-clap test-ui test-modgui test-all clean distclean

all: ttl

dpf/Makefile.plugins.mk:
	git submodule update --init --recursive

$(TTL_GEN): dpf/Makefile.plugins.mk
	$(MAKE) -C dpf/utils/lv2-ttl-generator

plugin: dpf/Makefile.plugins.mk
	$(MAKE) -C plugins/BluesFuse NOUI=$(NOUI)

# generate the LV2 TTL (incl. presets.ttl from the plugin programs) and
# add the static modgui
ttl: plugin $(TTL_GEN)
	cd $(BUNDLE) && $(CURDIR)/$(TTL_GEN) ./$(PLUG)_dsp.so
	# DPF prefixes audio port symbols with "lv2_"; restore the plugin's
	# original "in"/"out" (pedalboards store connections by symbol)
	sed -i -e 's/lv2:symbol "lv2_in"/lv2:symbol "in"/' \
	       -e 's/lv2:symbol "lv2_out"/lv2:symbol "out"/' $(BUNDLE)/$(PLUG)_dsp.ttl
	rm -rf $(BUNDLE)/modgui
	cp -r modgui $(BUNDLE)/modgui
	mv $(BUNDLE)/modgui/modgui.ttl $(BUNDLE)/modgui.ttl

tf:
	python3 tools/gen_tf.py > $(TF).tmp && mv $(TF).tmp $(TF)

dsp:
	python3 $(DSP2CC) --init-type=plugin-lv2 -double -o $(CC_GEN) $(DSP)

build/render_modgui: tools/render_modgui.cpp plugins/BluesFuse/Artwork.hpp
	mkdir -p build
	$(CXX) -O2 -std=c++17 -Wall -o $@ $< $$(pkg-config --cflags --libs cairo)

modgui: build/render_modgui
	build/render_modgui modgui

install: ttl
	mkdir -p "$(DESTDIR)$(INSTALL_DIR)"
	rm -rf "$(DESTDIR)$(INSTALL_DIR)/$(PLUG).lv2"
	cp -r $(BUNDLE) "$(DESTDIR)$(INSTALL_DIR)/"

install-all: install
	mkdir -p "$(DESTDIR)$(VST3_DIR)" "$(DESTDIR)$(CLAP_DIR)" "$(DESTDIR)$(BIN_DIR)"
	rm -rf "$(DESTDIR)$(VST3_DIR)/$(PLUG).vst3"
	cp -r bin/$(PLUG).vst3 "$(DESTDIR)$(VST3_DIR)/"
	cp bin/$(PLUG).clap "$(DESTDIR)$(CLAP_DIR)/"
	cp bin/$(PLUG) "$(DESTDIR)$(BIN_DIR)/$(PLUG)"

test: ttl
	mkdir -p build
	$(CC) -O2 -o build/bf_test tests/bf_test.c -ldl -lm
	build/bf_test $(CURDIR)/$(BUNDLE)/$(PLUG)_dsp.so

# CLAP: load through the factory, activate, process, list parameters
test-clap: ttl
	mkdir -p build
	$(CC) -O2 -std=c11 -Idpf/distrho/src -o build/clap_check tests/clap_check.c -ldl -lm
	build/clap_check bin/$(PLUG).clap pi.stom.dataforte.$(PLUG) 8

# Drives the real UI under Xvfb with XTest: host->UI values, knob drags,
# wheel, double-click reset, COMP lever, MODEL switch, footswitch, resizing.
# Ports: gain 2, volume 3, treble 4, bass 5, mid 6, comp 7, model 8,
# lv2_enabled 9.
UI_SHOTS ?= build/ui-shots
test-ui: ttl
	mkdir -p build $(UI_SHOTS)
	$(CC) -O2 -o build/ui_test tests/ui_test.c -ldl -lm -lX11 -lXtst
	xvfb-run -a -s "-screen 0 1400x1400x24" build/ui_test \
	  $(CURDIR)/$(BUNDLE)/$(PLUG)_ui.so http://dataforte.stom-pi/plugins/$(PLUG) \
	  $(CURDIR)/$(BUNDLE)/ 240x450 portmap:lv2_enabled=9 \
	  size:240x450 shot:$(UI_SHOTS)/default.png \
	  set:2=8 set:6=7 set:7=0 set:8=1 set:9=0 shot:$(UI_SHOTS)/host-values.png \
	  drag:68.75,78.75,-40 expect:2\>9.5 \
	  wheel:171.25,182.5,-5 expect:5\<4.5 \
	  drag:120,130,-60 expect:6\>9.5 dclick:120,130 expect:6=2 \
	  click:120,218 expect:7=1 click:120,218 expect:7=2 click:120,195 expect:7=1 \
	  click:120,322.5 expect:8=0 \
	  click:120,371.25 expect:9=1 \
	  resize:480x900 size:480x900 shot:$(UI_SHOTS)/480x900.png \
	  drag:68.75,182.5,40 expect:3\<3.5 \
	  resize:600x400 shot:$(UI_SHOTS)/600x400-letterbox.png \
	  click:120,371.25 expect:9=0

# Renders the modgui in headless Chrome at the default values and checks
# every control against the cairo artwork (needs google-chrome + PIL).
test-modgui: build/render_modgui
	build/render_modgui --preview build/modgui-ref.png 1.6 > /dev/null
	python3 tests/modgui_check.py modgui build build/modgui-ref.png \
	  gain=5:0:10 volume=5:0:10 treble=5:0:10 bass=5:0:10 mid=2:0:10 comp=2:0:2 model=0:0:1 led=on

test-all: test test-clap test-ui test-modgui

clean:
	$(MAKE) -C plugins/BluesFuse clean
	rm -rf bin build

distclean: clean
	$(MAKE) -C dpf clean
