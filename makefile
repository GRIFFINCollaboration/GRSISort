SUBDIRS = src libraries
ALLDIRS = $(SUBDIRS)

PLATFORM = $(shell uname)

export PLATFORM:= $(PLATFORM)

export CFLAGS = -std=c++0x -O2 -I$(PWD)/include

#export GRSISYS:= $(GRSISYS)

ifeq ($(PLATFORM),Darwin)
export __APPLE__:= 1
export CFLAGS += -DOS_DARWIN -std=c++11 -DHAVE_ZLIB #-lz
export CFLAGS += -m64 -I$(ROOTSYS)/include
export LFLAGS = -dynamiclib -undefined dynamic_lookup -single_module # 
export SHAREDSWITCH = -install_name # ENDING SPACE
export CPP = xcrun clang++ 
else
export __LINUX__:= 1	
export CFLAGS += `root-config --cflags`
export SHAREDSWITCH = -shared -Wl,-soname,#NO ENDING SPACE
export CPP = g++
endif
export COMPILESHARED   = $(CPP) $(LFLAGS) $(SHAREDSWITCH)#NO ENDING SPACE

export BASE:= $(CURDIR)

export CAT=cat

export OK_STRING="[OK]"
export ERROR_STRING="[ERROR]"
export WARN_STRING="[WARNING]"
export COMP_STRING="Now Compiling "
export FIN_STRING="Finished Building "

export COM_COLOR=\033[0;34m
export OBJ_COLOR=\033[0;36m
export DICT_COLOR=\033[0;36m
export OK_COLOR=\033[0;32m
export ERROR_COLOR=\033[0;31m
export WARN_COLOR=\033[0;33m
export NO_COLOR=\033[m
export FIN_COLOR=\033[3;34m
export FIN_OBJ_COLOR=\033[3;32m

MAKE=make --no-print-directory 

.PHONY: all subdirs $(ALLDIRS) clean

all: print subdirs bin grsihist grsisort html end

simple: print subdirs bin grsihist grsisort end

print:
	@echo "Compiling on $(PLATFORM)"

subdirs: $(SUBDIRS)

src: libraries

$(ALLDIRS):
	@$(MAKE) -C $@

grsisort: src
	@mv $</$@ bin/$@

bin:
ifeq ($(wildcard ./bin),) 
	@mkdir bin	 
endif

grsihist:
ifeq ($(wildcard ./.grsi_history),)
	@touch .grsi_history
endif

html: libraries grsisort
	@printf " ${COM_COLOR}Building      ${OBJ_COLOR} HTML Documentation ${NO_COLOR}\n"
	@root -b -q util/html_generator.C >/dev/null
	@$(RM) tempfile.out

end: grsisort
	@printf " ${WARN_COLOR}Compilation Success. woohoo!${NO_COLOR}\n\n"

clean:
	@$(RM) *~
	$(RM) ./bin/grsisort
	@for dir in $(ALLDIRS); do \
		$(MAKE) -C $$dir $@; \
	done

veryclean: clean
	$(RM) -r ./htmldoc


