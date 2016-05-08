# -*- Makefile -*-
# Example standalone makefile for foreachmare

pms_SOURCES = filtro-mare.cc

pms_OBJECTS = $(pms_SOURCES:%.cc=%.o)

# Same value as provided to MARE for the invocation of ./configure --prefix=...
MARE_PREFIX = /home/mizadri/Qualcomm/MARE/0.12.1/x86_64-linux-gnu/debug

CXX= /usr/bin/g++-4.8 -std=c++11 -pthread
CPPFLAGS = -I$(MARE_PREFIX)/include
CXXFLAGS = -O2 -Wall -Wextra
LDFLAGS = -L$(MARE_PREFIX)/lib
LIBS= -lmare

all: pms

pms: $(pms_OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@

.PHONY: clean
clean:
	$(RM) $(pms_OBJECTS) pms

.DELETE_ON_ERROR:
