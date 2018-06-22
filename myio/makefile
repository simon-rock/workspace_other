ROOT = .
CC = g++ -g -c -Wno-deprecated
BINDIR = $(ROOT)/bin/
LIBDIR = $(ROOT)/lib/

LINK	= g++
LINKSO  = g++ -shared
#CC	= g++ -g -c 
AR	= ar -cr
RM	= rm -rf

# Target
TARGETS	= $(TARGETS1) $(TARGETS2) $(TARGETS3) $(TARGETS4) 
TARGETS1= libnull.so
TARGETS2= 
TARGETS3= 
TARGETS4= 
#Object
OBJS	= $(OBJ_T1)
OBJ_T1	= null.o
OBJ_T2  = 
OBJ_T3  = 
OBJ_T4  = 
OBJ_T5  = 
OBJ_T6  = 
OBJ_T7  = 

# include
INCS = -I../fio
COMPATIBILITY = -fpermissive -DFIO_EXTERNAL_ENGINE 
OPTIONS	= -ftemplate-depth-100 -finline-functions -Wno-inline -Wall -Wsign-compare -fPIC
CFLAGS	= $(INCS) $(OPTIONS) $(COMPATIBILITY)
# lib
LIBS = -ltbsys -ltbnet -lpthread -lcurl 
# libs depend
LIBDIRS	=


all: $(TARGETS)

$(TARGETS1): libs $(OBJ_T1) 
#	$(LINK) -o $(BINDIR)$(TARGETS1)  $(OBJ_T1) $(LIBS)
#	$(AR)  $(BINDIR)$(TARGETS1)  $(OBJ_T1)
	$(LINKSO) -o $(BINDIR)$(TARGETS1)  $(OBJ_T1) 

libs:
	@$(foreach BIND, $(LIBDIRS), \
            $(MAKE) -C $(BIND) -f Makefile --no-print-directory;)
.cpp.o	:
	$(CC)  $(CFLAGS) $< 

clean:
	@$(foreach BIND, $(LIBDIRS), \
            $(MAKE) -C $(BIND) -f Makefile clean --no-print-directory;)
	@$(foreach T, $(TARGETS), \
            $(RM) $(BINDIR)$(T);)
	-$(RM) $(OBJS) *.o *core

.PHONY: all clean dependents
