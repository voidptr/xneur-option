PROGRAM = loggerd

INCLUDES = -Iconnections -Itools -Iio -Iworkers

CXX = g++
CXXALLFLAGS = ${CXXFLAGS} -Wall -Wextra -g0 -pedantic -fPIC

CXXCOMPILE = ${CXX} ${CXXALLFLAGS} ${INCLUDES}
CXXLINK = ${CXX} ${CXXALLFLAGS}

SUBDIRS = io workers connections tools .

OBJECTS = io/net_io.o workers/worker.o workers/worker_fork.o workers/workers_pool.o connections/connection.o connections/connection_kqueue.o connections/connection_select.o tools/functions.o tools/descriptors_transmitter.o loggerd.o

HEADERS = io/net_io.h workers/worker.h workers/worker_fork.h workers/workers_pool.h connections/connection.h connections/connection_select.h connections/connection_kqueue.h tools/functions.h tools/descriptors_transmitter.h io/defines.h

all: header ${PROGRAM} footer

.SUFFIXES:
.SUFFIXES: .cpp .o

header:
	@echo "Using c++ compiler: ${CXX}"
	@echo "Compile flags:      ${CXXALLFLAGS}"
	@echo

footer:
	@echo
	@echo "Build done"

${PROGRAM}: ${OBJECTS}
	@echo "	[L] ${PROGRAM}"
	@${CXXLINK} -o $@ ${OBJECTS}

	@echo "	[S] ${PROGRAM}"
	@strip ${PROGRAM}

.cpp.o: Makefile ${HEADERS}
	@echo "	[C] $<"
	@${CXXCOMPILE} -c -o $@ $<

clean:
	@for dir in ${SUBDIRS}; do \
		echo "	[C] $$dir"; \
		rm -f $$dir/*.o; \
	done

	@echo "	[D] ${PROGRAM}"
	@rm -f ${PROGRAM}

cleantemp:
	@for dir in ${SUBDIRS}; do \
		echo "	[C] $$dir"; \
		rm -f $$dir/*~; \
	done


.PHONY: all clean cleantemp footer header

.NOEXPORT:
