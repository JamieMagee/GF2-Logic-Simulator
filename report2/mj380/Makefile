all: src/devices.cc.diff src/network.cc.diff src/userint.cc.diff src/devices.h.diff src/logsim.h.diff src/network.h.diff src/circuit.cc src/error.cc src/gui-canvas.cc src/gui.cc src/gui-devices.cc src/gui-devices-infopanels.cc src/gui-misc.cc src/guitest.cc src/logsim.cc src/monitor.cc src/names.cc src/observer-test.cc src/parser.cc src/runtests.cc src/scanner.cc src/tests.cc src/circuit.h src/error.h src/gui-canvas.h src/gui-devices.h src/gui-devices-infopanels.h src/gui.h src/gui-id.h src/gui-misc.h src/guitest.h src/monitor.h src/names.h src/observer.h src/parser.h src/scanner.h src/tests.h src/userint.h

# git branch -D indent;git checkout -b indent;~/bin/astyle -oOt *.cc *.h;rm *.orig;git add .;git commit -m 'Indentation';git checkout master;

clean:
	rm -f src/*.diff src/*.cc src/*.h

src/%.diff: ../../src/%
	git diff -w originalsrc indent -- $< | tail -n +5 > $@

src/%.cc: ../../src/%.cc
	cp $< $@
	~/bin/astyle -oOt $@
	rm -f $@.orig

src/%.h: ../../src/%.h
	cp $< $@
	~/bin/astyle -oOt $@
	rm -f $@.orig


