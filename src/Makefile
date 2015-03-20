OPENGL_LIBS = -lglut -lGL -lGLU

CFLAGS = -Wall -Werror -Wextra -pedantic -std=c++1y 

SRC = $(wildcard *.cc)

L_OBJECTS = logsim.o names.o scanner.o network.o parser.o monitor.o devices.o userint.o gui.o circuit.o error.o gui-devices.o gui-misc.o gui-devices-infopanels.o gui-canvas.o gui-options.o

G_OBJECTS = guitest.o names.o network.o monitor.o devices.o gui.o scanner.o parser.o

T_OBJECTS = runtests.o tests.o names.o scanner.o

# implementation

.SUFFIXES:	.o .cc

.cc.o :
	$(CXX) -c `wx-config --cxxflags` $(CFLAGS) -g -o $@ $<

all:    logsim guitest

logsim:	$(L_OBJECTS)
	$(CXX) -o logsim $(CFLAGS) $(L_OBJECTS) `wx-config --libs --gl_libs` $(OPENGL_LIBS)

guitest: $(G_OBJECTS)
	 $(CXX) -o guitest $(CFLAGS) $(G_OBJECTS) `wx-config --libs --gl_libs` $(OPENGL_LIBS)

runtests: $(T_OBJECTS)
	$(CXX) -o runtests $(CFLAGS) $(T_OBJECTS) `wx-config --libs --gl_libs` $(OPENGL_LIBS)

observer-test: observer-test.cc observer.h

clean: 
	rm -f *.o logsim guitest runtests observer-test

depend:
	# -Y stops it adding files in /usr/include, just to keep the Makefile a bit tidier since those files won't change very often.
	# Do "make clean" if /usr/include/* files change.
	makedepend -Y $(SRC)

# DO NOT DELETE

circuit.o: circuit.h names.h network.h devices.h monitor.h observer.h
devices.o: devices.h names.h network.h monitor.h
error.o: error.h scanner.h names.h
gui-canvas.o: gui-canvas.h circuit.h names.h network.h devices.h monitor.h
gui-canvas.o: observer.h gui-misc.h gui-options.h
gui.o: gui.h gui-canvas.h circuit.h names.h network.h devices.h monitor.h
gui.o: observer.h gui-misc.h gui-options.h wx_icon.xpm scanner.h parser.h
gui.o: error.h gui-devices.h gui-devices-infopanels.h gui-id.h
gui-devices.o: gui-devices.h gui-devices-infopanels.h circuit.h names.h
gui-devices.o: network.h devices.h monitor.h observer.h gui-misc.h gui-id.h
gui-devices.o: wx_icon.xpm
gui-devices-infopanels.o: gui-devices-infopanels.h circuit.h names.h
gui-devices-infopanels.o: network.h devices.h monitor.h observer.h gui-misc.h
gui-devices-infopanels.o: gui-devices.h gui-id.h
gui-misc.o: gui-misc.h circuit.h names.h network.h devices.h monitor.h
gui-misc.o: observer.h
gui-options.o: gui-options.h observer.h
guitest.o: guitest.h names.h devices.h network.h monitor.h gui.h gui-canvas.h
guitest.o: circuit.h observer.h gui-misc.h gui-options.h
logsim.o: logsim.h names.h devices.h network.h monitor.h parser.h scanner.h
logsim.o: error.h userint.h gui.h gui-canvas.h circuit.h observer.h
logsim.o: gui-misc.h gui-options.h
monitor.o: monitor.h names.h network.h devices.h
names.o: names.h
network.o: network.h names.h
observer-test.o: observer.h
parser.o: parser.h names.h scanner.h network.h devices.h monitor.h error.h
runtests.o: tests.h scanner.h names.h
scanner.o: scanner.h names.h
tests.o: tests.h scanner.h names.h
userint.o: userint.h names.h network.h devices.h monitor.h
