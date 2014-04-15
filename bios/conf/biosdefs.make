CC = gcc
CFLAGS = -m64 -Wall -Wno-parentheses -Wno-sign-compare -Wno-unknown-pragmas -g
CFLAGSO = -m64 -Wall -Wno-parentheses -Wno-sign-compare -Wno-unknown-pragmas -Wno-uninitialized -g -O2
CCEXPAND = $(CC) -x c -E
CPP = /usr/bin/cpp

BIOSLIBLOC = $(BIOINFOCONFDIR)/../lib

BIOSOBJ = $(BIOINFOCONFDIR)/../obj

BICOSINC = -I$(BIOINFOCONFDIR)
BIOSINC = $(BICOSINC) -I$(BIOSLIBLOC)

BIOSLIB = $(BIOSLIBLOC)/libbios.a
BIOSLNK = $(BIOSLIBLOC)/libbios.a
