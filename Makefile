# Makefile for 

lib.name = grid

class.sources = grid.c step.c

PDINCLUDEDIR=/usr/local/include/pd

ldflags = -llo -fno-common -g
cflags+= -I./dummy_include -g

#datafiles = myclass1-help.pd myclass2-help.pd README.txt LICENSE.txt

common.sources = net_monome.c

include ../pd-lib-builder/Makefile.pdlibbuilder

tags:
	find . ~/git_checkouts/pure-data -name '*.[ch]' | etags -

test:
	cp grid.pd_linux ~/pd-externals; pd grid_example.pd
