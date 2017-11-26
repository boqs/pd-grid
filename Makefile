# Makefile for 

lib.name = grid

class.sources = grid.c\
	step.c \
	ww.c \
	kria.c \
	mp.c

PDINCLUDEDIR=/usr/local/include/pd

ldflags = -llo -fno-common -g
cflags+= -I./dummy_include -g

datafiles = grid-help.pd README.txt LICENSE.txt

common.sources = net_monome.c

include Makefile.pdlibbuilder

tags:
	find . ~/git_checkouts/pure-data -name '*.[ch]' | etags -

test:
	cp *.pd_linux ~/pd-externals; pd grid-example.pd

local-install:
	cp *.pd_linux ~/pd-externals
