.PHONY: clean all docs doxygen
.SECONDARY:
.SECONDEXPANSION:


PLATFORM:=$(shell uname)
# EDIT THIS SECTION

INCLUDES   = include users
CFLAGS     = -g -std=c++0x -O3 -Wall -Wextra -pedantic -Wno-unused-parameter  -Wno-unused-function
#-Wall -Wextra -pedantic -Wno-unused-parameter
LINKFLAGS_PREFIX  =
LINKFLAGS_SUFFIX  = -L/opt/X11/lib -lX11 -lXpm -std=c++0x
SRC_SUFFIX = cxx

# EVERYTHING PAST HERE SHOULD WORK AUTOMATICALLY

MAJOR_ROOT_VERSION:=$(shell root-config --version | cut -d '.' -f1)

CFLAGS += -DMAJOR_ROOT_VERSION=${MAJOR_ROOT_VERSION} 

ifeq ($(PLATFORM),Darwin)
export __APPLE__:= 1
CFLAGS     += -DOS_DARWIN -DHAVE_ZLIB
CFLAGS     += -I/opt/X11/include -Qunused-arguments
CPP        = clang++
SHAREDSWITCH = -Qunused-arguments -shared -undefined dynamic_lookup -dynamiclib -install_name,'@executable_path/../libraries/$$@'# NO ENDING SPACE
else
export __LINUX__:= 1
CPP        = g++
CFLAGS     += -Wl,--no-as-needed
LINKFLAGS_PREFIX += -Wl,--no-as-needed
SHAREDSWITCH = -shared -Wl,-soname,# NO ENDING SPACE
endif

COM_COLOR=\033[0;34m
OBJ_COLOR=\033[0;36m
BLD_COLOR=\033[3;34m
LIB_COLOR=\033[3;36m
OK_COLOR=\033[0;32m
ERROR_COLOR=\033[0;31m
WARN_COLOR=\033[0;33m
NO_COLOR=\033[m

OK_STRING="[OK]"
ERROR_STRING="[ERROR]"
WARN_STRING="[WARNING]"
COM_STRING="Compiling"
BLD_STRING="Building\ "
FIN_STRING="Finished Building"

LIBRARY_DIRS   := $(shell find libraries/* -type d 2> /dev/null | grep -v SourceData)
LIBRARY_NAMES  := $(notdir $(LIBRARY_DIRS))
LIBRARY_OUTPUT := $(patsubst %,libraries/lib%.so,$(LIBRARY_NAMES))

INCLUDES  := $(addprefix -I$(PWD)/,$(INCLUDES))
CFLAGS    += $(shell root-config --cflags)
CFLAGS    += -MMD $(INCLUDES)
LINKFLAGS += -Llibraries $(addprefix -l,$(LIBRARY_NAMES)) -Wl,-rpath,\$$ORIGIN/../libraries
LINKFLAGS += $(shell root-config --glibs) -lSpectrum -lXMLParser -lXMLIO -lGuiHtml -lTreePlayer -lX11 -lXpm -lProof
LINKFLAGS := $(LINKFLAGS_PREFIX) $(LINKFLAGS) $(LINKFLAGS_SUFFIX) $(CFLAGS)

ROOT_LIBFLAGS := $(shell root-config --cflags --glibs)

UTIL_O_FILES    := $(patsubst %.$(SRC_SUFFIX),.build/%.o,$(wildcard util/*.$(SRC_SUFFIX)))
#SANDBOX_O_FILES := $(patsubst %.$(SRC_SUFFIX),.build/%.o,$(wildcard Sandbox/*.$(SRC_SUFFIX)))
SCRIPT_O_FILES    := $(patsubst %.$(SRC_SUFFIX),.build/%.o,$(wildcard scripts/*.$(SRC_SUFFIX)))
ANALYSIS_O_FILES := $(patsubst %.$(SRC_SUFFIX),.build/%.o,$(wildcard myAnalysis/*.$(SRC_SUFFIX)))
MAIN_O_FILES    := $(patsubst %.$(SRC_SUFFIX),.build/%.o,$(wildcard src/*.$(SRC_SUFFIX)))
EXE_O_FILES     := $(UTIL_O_FILES) $(SANDBOX_O_FILES) $(SCRIPT_O_FILES) $(ANALYSIS_O_FILES)
EXECUTABLES     := $(patsubst %.o,bin/%,$(notdir $(EXE_O_FILES))) bin/grsisort

run_and_test =@printf "%b%b%b" " $(3)$(4)$(5)" $(notdir $(2)) "$(NO_COLOR)\r";  \
                $(1) 2> $(2).log || touch $(2).error; \
                if test -e $(2).error; then \
                      printf "%-60b%b%s%b" "$(3)$(4)$(5)$(2)" "$(ERROR_COLOR)" "$(ERROR_STRING)" "$(NO_COLOR)\n"   ; \
                      cat $(2).log; \
                      rm -f $(2).log $(2).error; \
                      exit 1; \
                elif test -s $(2).log; then \
                      printf "%-60b%b%s%b" "$(3)$(4)$(5)$(2)" "$(WARN_COLOR)" "$(WARN_STRING)" "$(NO_COLOR)\n"   ; \
                      cat $(2).log; \
                else  \
                      printf "%b%-60s%b%s%b" "$(3)$(4)$(5)" $(notdir $(2)) "$(OK_COLOR)" "$(OK_STRING)" "$(NO_COLOR)\n"   ; \
                fi; \
                rm -f $(2).log $(2).error

all: $(EXECUTABLES) $(LIBRARY_OUTPUT) config 
	@find .build users -name "*.pcm" -exec cp {} libraries/ \;
	@printf "$(OK_COLOR)Compilation successful, $(WARN_COLOR)woohoo!$(NO_COLOR)\n"

docs: doxygen

doxygen:
	$(MAKE) -C $@

bin/grsisort: $(MAIN_O_FILES) | $(LIBRARY_OUTPUT) bin
	$(call run_and_test,$(CPP) $^ -o $@ $(LINKFLAGS),$@,$(COM_COLOR),$(COM_STRING),$(OBJ_COLOR) )

bin/%: .build/util/%.o | $(LIBRARY_OUTPUT) bin
	$(call run_and_test,$(CPP) $< -o $@ $(LINKFLAGS),$@,$(COM_COLOR),$(COM_STRING),$(OBJ_COLOR) )

bin/%: .build/Sandbox/%.o | $(LIBRARY_OUTPUT) bin
	$(call run_and_test,$(CPP) $< -o $@ $(LINKFLAGS),$@,$(COM_COLOR),$(COM_STRING),$(OBJ_COLOR) )

bin/%: .build/scripts/%.o | $(LIBRARY_OUTPUT) bin
	$(call run_and_test,$(CPP) $< -o $@ $(LINKFLAGS),$@,$(COM_COLOR),$(COM_STRING),$(OBJ_COLOR) )

bin/%: .build/myAnalysis/%.o | $(LIBRARY_OUTPUT) bin
	$(call run_and_test,$(CPP) $< -o $@ $(LINKFLAGS),$@,$(COM_COLOR),$(COM_STRING),$(OBJ_COLOR) )

bin:
	@mkdir -p $@

config:
	@cp util/grsi-config bin/

# Functions for determining the files included in a library.
# All src files in the library directory are included.
# If a LinkDef.h file is present in the library directory,
#    a dictionary file will also be generated and added to the library.
libdir          = $(shell find libraries -name $(1) -type d)
lib_src_files   = $(shell find $(call libdir,$(1)) -name "*.$(SRC_SUFFIX)")
lib_o_files     = $(patsubst %.$(SRC_SUFFIX),.build/%.o,$(call lib_src_files,$(1)))
lib_linkdef     = $(wildcard $(call libdir,$(1))/LinkDef.h)
lib_dictionary  = $(patsubst %/LinkDef.h,.build/%/LibDictionary.o,$(call lib_linkdef,$(1)))

libraries/lib%.so: $$(call lib_o_files,%) $$(call lib_dictionary,%)
	$(call run_and_test,$(CPP) -fPIC $^ $(SHAREDSWITCH)lib$*.so $(ROOT_LIBFLAGS) -o $@,$@,$(BLD_COLOR),$(BLD_STRING),$(OBJ_COLOR) )

.build/%.o: %.$(SRC_SUFFIX)
	@mkdir -p $(dir $@)
	$(call run_and_test,$(CPP) -fPIC -c $< -o $@ $(CFLAGS),$@,$(COM_COLOR),$(COM_STRING),$(OBJ_COLOR) )

dict_header_files = $(addprefix $(PWD)/include/,$(subst //,,$(shell head -n 1 $(1) 2> /dev/null)))
find_linkdef = $(shell find $(1) -name "*LinkDef.h")

# In order for all function names to be unique, rootcint requires unique output names.
# Therefore, usual wildcard rules are insufficient.
# Eval is more powerful, but is less convenient to use.
define library_template
.build/$(1)/$(notdir $(1))Dict.cxx: $(1)/LinkDef.h $$(call dict_header_files,$(1)/LinkDef.h)
	@mkdir -p $$(dir $$@)
	$$(call run_and_test,rootcint -f $$@ -c $$(INCLUDES) -p $$(notdir $$(filter-out $$<,$$^)) $$<,$$@,$$(COM_COLOR),$$(BLD_STRING) ,$$(OBJ_COLOR))

.build/$(1)/LibDictionary.o: .build/$(1)/$(notdir $(1))Dict.cxx
	$$(call run_and_test,$$(CPP) -fPIC -c $$< -o $$@ $$(CFLAGS),$$@,$$(COM_COLOR),$$(COM_STRING),$$(OBJ_COLOR) )
endef

$(foreach lib,$(LIBRARY_DIRS),$(eval $(call library_template,$(lib))))

-include $(shell find .build -name '*.d' 2> /dev/null)

html: all
	@printf " ${COM_COLOR}Building      ${OBJ_COLOR} HTML Documentation ${NO_COLOR}\n"
	@cp -r include grsisort
	@grsisort -q -l --work_harder util/html_generator.C #>/dev/null
	@$(RM) -r grsisort
	@$(RM) tempfile.out

clean:
	@printf "\nCleaning up\n\n"
	@-$(RM) -rf .build
	@-$(RM) -rf bin
	@-$(RM) -f $(LIBRARY_OUTPUT)
	@-$(RM) -f libraries/*.pcm

cleaner: clean
	@printf "\nEven more clean up\n\n"
	@-$(RM) -rf htmldoc
