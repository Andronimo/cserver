#VERSION = 3.02
VERBOSE  = 0
CXX      = gcc

INC_DIR += -I/usr/include/glib-2.0/
INC_DIR += -I/usr/lib/x86_64-linux-gnu/glib-2.0/include/ 
INC_DIR += -I./inc/base/
INC_DIR += -I./inc/chtml/
INC_DIR += -I./inc/
INC_DIR += -I./elib/

CPPFLAGS  = -Wall -Wno-write-strings -g3 -O2 -D_REENTRANT -DVERSION=\"$(VERSION)\" $(INC_DIR)
LDFLAGS = -Wl,-rpath,elib -Lelib -lglib-2.0 -lcrypto -lssl -lsqlite3 -ltools 

OBJDIR=obj
BINDIR=bin
CHTMLDIR=src/chtml
CHTMLINCDIR=inc/chtml

include inputs.mk		
-include $(OBJ:.o=.d)

.PHONY: clean includes check folders elib

.DEFAULT_GOAL :=
all: elib folders $(BINDIR)/server 

folders:
	@mkdir -p $(CHTMLDIR) $(OBJDIR)/chtml $(OBJDIR)/base $(BINDIR) $(CHTMLINCDIR)

elib:
	@$(MAKE) -C $@

inputs:
	@find src | grep '\.c$$' | sed '/src\/chtml/ d' | sed 's/src\/\(.*\)\.c/OBJ+=obj\/\1.o/' > inputs.mk 	
	@find html | grep '\.html$$' | sed 's/html\/\(.*\)\.html/HTML_OBJ+=src\/chtml\/\1.c\nOBJ+=obj\/chtml\/\1.o/' >> inputs.mk 	
	@cat inputs.mk
	
$(CHTMLDIR)/%.c: html/%.html
	@tools/prepareHeader.py $<
	@tools/prepareFile.py $<

$(OBJDIR)/%.o: src/%.c
	@echo "compiling.." $<
	@$(CXX) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
	@$(CXX) -MM $(CFLAGS) $(CPPFLAGS) $< > $(OBJDIR)/$*.d
	@sed -i '1 s/^/obj\//' $(OBJDIR)/$*.d

$(BINDIR)/server: $(HTML_OBJ) $(OBJ)
	@echo "linking.."
	@$(CXX) $(CPPFLAGS) -o $@ $(OBJ) $(LDFLAGS)

clean:
	@$(MAKE) -C elib clean 
	rm -rf $(BINDIR) $(OBJDIR) $(CHTMLDIR) $(CHTMLINCDIR)
	
check:
	@echo checking...
	@cppcheck --enable=all src/*
