OPENGL_LIBS = -lglut -lGL -lGLU

CXX = $(shell wx-config --cxx)

SRC = logsim.cc names.cc scanner.cc network.cc parser.cc monitor.cc devices.cc userint.cc gui.cc guitest.cc

L_OBJECTS = logsim.o names.o scanner.o network.o parser.o monitor.o devices.o userint.o gui.o circuit.o

G_OBJECTS = guitest.o names.o network.o monitor.o devices.o gui.o scanner.o parser.o

T_OBJECTS = runtests.o tests.o names.o scanner.o

# implementation

.SUFFIXES:	.o .cc

.cc.o :
	$(CXX) -c `wx-config --cxxflags` -g -o $@ $<

all:    logsim guitest

logsim:	$(L_OBJECTS)
	$(CXX) -o logsim $(L_OBJECTS) `wx-config --libs --gl_libs` $(OPENGL_LIBS)

guitest: $(G_OBJECTS)
	 $(CXX) -o guitest $(G_OBJECTS) `wx-config --libs --gl_libs` $(OPENGL_LIBS)

runtests: $(T_OBJECTS)
	$(CXX) -o runtests $(T_OBJECTS) `wx-config --libs --gl_libs` $(OPENGL_LIBS)

observer-test: observer-test.cc observer.h

clean: 
	rm -f *.o logsim guitest runtests

depend:
	# -Y stops it adding files in /usr/include, just to keep the Makefile a bit tidier since those files won't change very often.
	# Do "make clean" if /usr/include/* files change.
	makedepend -Y $(SRC)

# DO NOT DELETE

devices.o: devices.h names.h network.h
gui.o: gui.h names.h devices.h network.h monitor.h wx_icon.xpm circuit.h observer.h
guitest.o: guitest.h names.h devices.h network.h monitor.h gui.h
logsim.o: logsim.h names.h devices.h network.h monitor.h parser.h scanner.h
logsim.o: userint.h gui.h
monitor.o: monitor.h names.h network.h devices.h
names.o: names.h
network.o: network.h names.h
parser.o: parser.h names.h scanner.h network.h devices.h monitor.h
scanner.o: scanner.h names.h
userint.o: userint.h names.h network.h devices.h monitor.h
circuit.o: circuit.cc circuit.h observer.h
