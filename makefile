SUBDIRS = src libraries
ALLDIRS = $(SUBDIRS) util examples scripts users

PLATFORM = $(shell uname)

export PLATFORM:= $(PLATFORM)

export MAJOR_ROOT_VERSION = `root-config --version | cut -d '.' -f1`
#if [ ${MAJOR_ROOT_VERSION} -lt 5 ] ; then \
#	$(error ${MAJOR_ROOT_VERSION} too small)
#fi

export CFLAGS = -std=c++0x -O2  -I$(PWD)/include -g `root-config --cflags` -DMAJOR_ROOT_VERSION=${MAJOR_ROOT_VERSION}

#export GRSISYS:= $(GRSISYS)

ifeq ($(PLATFORM),Darwin)
export __APPLE__:= 1
export CFLAGS += -DOS_DARWIN -DHAVE_ZLIB #-lz
export CFLAGS += -I/opt/X11/include -Qunused-arguments
export LFLAGS = -dynamiclib -undefined dynamic_lookup -single_module -Wl,-install_name,'@executable_path/../libraries/$$@'
export SHAREDSWITCH = -install_name # ENDING SPACE
export CXX = clang++ 
export CPP = clang++ 
export CXX = clang++
else
export __LINUX__:= 1	
export LFLAGS = -Wl,--no-as-needed
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

.PHONY: all subdirs $(ALLDIRS) clean util

#all: print config subdirs bin grsihist grsisort analysis util end

all: print grsisort analysis util end

docs: print subdirs bin grsihist grsisort html config end

util: libraries users grsisort print
	@$(MAKE) -C $@

examples: libraries users grsisort print
	@$(MAKE) -C $@

users: libraries print
	@$(MAKE) -C $@

scripts: libraries grsisort print
	@$(MAKE) -C $@

analysis: libraries users grsisort print
	@$(MAKE) -C myanalysis

print:
	@echo "Compiling on $(PLATFORM)"

subdirs: $(SUBDIRS)

src: print libraries users

$(SUBDIRS): print
	@$(MAKE) -C $@

grsisort: src libraries users print bin config
	@mv $</$@ bin/$@

config: print
	@cp util/grsi-config bin/
	@find libraries/*/ -name "*.pcm" -exec cp {} libraries/ \;

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
	@cp -r include grsisort
	@grsisort -q -l --work_harder util/html_generator.C #>/dev/null
	@$(RM) -r grsisort
	@$(RM) tempfile.out

end: grsisort
	@printf " ${WARN_COLOR}Compilation Success. woohoo!${NO_COLOR}\n\n"

clean:
	@$(RM) $(GRSISYS)/*~                      
	$(RM) -R $(GRSISYS)/bin/*dSYM
	$(RM) $(GRSISYS)/bin/*
	@for dir in $(ALLDIRS); do \
		$(MAKE) -C $$dir $@; \
	done

veryclean: clean
	$(RM) -r $(GRSISYS)/htmldoc


