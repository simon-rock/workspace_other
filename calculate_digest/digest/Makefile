#
# Copyright (c) 2012 by gaoyu. All Rights Reserved.
#
#ROOT := .
#include ../Makefile_s

LINK	= gcc
CC	= gcc -g -c 
AR	= ar -cr
RM	= rm -rf

# Target
TARGETS	= $(TARGETS1) $(TARGETS2)
TARGETS1= libdigest.a
TARGETS2= 

# OBJECT
OBJS	= $(OBJ_T1) $(OBJ_T2)
OBJ_T1	= md5_dgst.o sha1dgst.o mem_clr.o sha256.o sha512.o 
OBJ_T2	= 
# DIR
BINDIR	= ../../lib/
LIBDIR  = ../../lib/
# CFLAGS
INCS	=  
#LIBS	= -L ../lib  -lsystools -ldb_access  -L/usr/lib64/mysql/ -lmysqlclient #depend error
LIBS	= -L $(LIBDIR)
OPTIONS	= -ftemplate-depth-100 -O2 -finline-functions -Wno-inline -Wall 
COPTIONS	= -O2 -finline-functions -Wno-inline -Wall 
CFLAGS	= $(INCS) $(LIBS) $(OPTIONS)

# libs depend
LIBDIRS	= 

all: $(TARGETS)

$(TARGETS1): libs $(OBJ_T1) 
#	$(LINK) -o $(BINDIR)$(TARGETS1)  $(OBJ_T1) $(LIBS)
	$(AR)  $(BINDIR)$(TARGETS1)  $(OBJ_T1)
$(TARGETS2): libs $(OBJ_T2) 
	$(LINK) -o $(BINDIR)$(TARGETS2)  $(OBJ_T2) $(LIBS)
libs:
	@$(foreach BIND, $(LIBDIRS), \
            $(MAKE) -C $(BIND) -f Makefile --no-print-directory;)
.cpp.o	:
	$(CC)  $(INCS) $(OPTIONS) $< 
.c.o :
	$(CC)  $(INCS) $(COPTIONS) $<
clean:
	@$(foreach BIND, $(LIBDIRS), \
            $(MAKE) -C $(BIND) -f Makefile clean --no-print-directory;)
	@$(foreach T, $(TARGETS), \
            $(RM) $(BINDIR)$(T);)
	-$(RM) $(OBJS) *.o *core

.PHONY: all clean dependents

