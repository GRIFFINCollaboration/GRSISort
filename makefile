.PHONY: clean all extras docs doxygen grsirc complete parsers GRSIData ILLData iThembaData HILData
.SECONDARY:
.SECONDEXPANSION:


PLATFORM:=$(shell uname)
# EDIT THIS SECTION

INCLUDES   = include users
ifneq (,$(findstring -std=,$(shell root-config --cflags)))
CFLAGS = -g -O3 -Wall -Wextra -pedantic -Wno-unknown-pragmas -Wno-unused-function -Wshadow
LINKFLAGS_SUFFIX  = -L/opt/X11/lib -lX11 -lXpm
else
CFLAGS = -std=c++11 -g -O3 -Wall -Wextra -pedantic -Wno-unknown-pragmas -Wno-unused-function -Wshadow
LINKFLAGS_SUFFIX  = -std=c++11 -L/opt/X11/lib -lX11 -lXpm
endif
#-Wall -Wextra -pedantic -Wno-unused-parameter
LINKFLAGS_PREFIX  =
SRC_SUFFIX = cxx

# EVERYTHING PAST HERE SHOULD WORK AUTOMATICALLY

ROOT_PYTHON_VERSION=$(shell root-config --python-version)

MATHMORE_INSTALLED:=$(shell root-config --has-mathmore)
XML_INSTALLED:=$(shell root-config --has-xml)

ifeq ($(ROOT_PYTHON_VERSION),2.7)
  CFLAGS += -DHAS_CORRECT_PYTHON_VERSION
endif

ifeq ($(PLATFORM),Darwin)
export __APPLE__:= 1
CFLAGS     += -DOS_DARWIN -DHAVE_ZLIB
CFLAGS     += -I/opt/X11/include -Qunused-arguments -I/opt/local/include
CPP        = clang++
SHAREDSWITCH = -Qunused-arguments -shared -undefined dynamic_lookup -dynamiclib -Wl,-install_name,'@executable_path/../lib/'# NO ENDING SPACE
HEAD=ghead
FIND=gfind
LIBRARY_DIRS   := $(shell $(FIND) libraries/* -type d)
else
export __LINUX__:= 1
CPP        = g++
CFLAGS     += -Wl,--no-as-needed
LINKFLAGS_PREFIX += -Wl,--no-as-needed
SHAREDSWITCH = -shared -Wl,-soname,# NO ENDING SPACE
HEAD=head
FIND=find
LIBRARY_DIRS   := $(shell $(FIND) libraries/* -type d -maxdepth 2 2> /dev/null | grep -v SourceData | grep -v SRIMData)
endif

ROOTCINT=$(shell command -v rootcint 2> /dev/null)
ifndef $(ROOTCINT)
	ROOTCINT=rootcling
else
	ROOTCINT=rootcint
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
COM_STRING= "Compiling"
BLD_STRING= "Building\ "
COPY_STRING="Copying\ \ "
FIN_STRING="Finished Building"

LIBRARY_NAMES  := $(notdir $(LIBRARY_DIRS))
LIBRARY_OUTPUT := $(patsubst %,lib/lib%.so,$(LIBRARY_NAMES))

INCLUDES  := $(addprefix -I$(PWD)/,$(INCLUDES))
CFLAGS    += $(shell root-config --cflags)
CFLAGS    += -MMD -MP $(INCLUDES)
LINKFLAGS += -Llib $(addprefix -l,$(LIBRARY_NAMES)) -Wl,-rpath,\$$ORIGIN/../lib
LINKFLAGS += $(shell root-config --glibs) -lSpectrum -lMinuit -lGuiHtml -lTreePlayer -lX11 -lXpm -lProof -lTMVA

# RCFLAGS are being used for rootcint
ifeq ($(MATHMORE_INSTALLED),yes)
  CFLAGS += -DHAS_MATHMORE
  RCFLAGS += -DHAS_MATHMORE
  LINKFLAGS += -lMathMore
endif

ifeq ($(XML_INSTALLED),yes)
  CFLAGS += -DHAS_XML
  RCFLAGS += -DHAS_XML
  LINKFLAGS += -lXMLParser -lXMLIO
endif

LINKFLAGS := $(LINKFLAGS_PREFIX) $(LINKFLAGS) $(LINKFLAGS_SUFFIX) $(CFLAGS)

ROOT_LIBFLAGS := $(shell root-config --cflags --glibs)

UTIL_O_FILES    := $(patsubst %.$(SRC_SUFFIX),.build/%.o,$(wildcard util/*.$(SRC_SUFFIX)))
#SANDBOX_O_FILES := $(patsubst %.$(SRC_SUFFIX),.build/%.o,$(wildcard Sandbox/*.$(SRC_SUFFIX)))
SCRIPT_O_FILES    := $(patsubst %.$(SRC_SUFFIX),.build/%.o,$(wildcard scripts/*.$(SRC_SUFFIX)))
PROOF_O_FILES    := $(patsubst %.$(SRC_SUFFIX),.build/%.o,$(wildcard GRSIProof/grsiproof.$(SRC_SUFFIX)))
ANALYSIS_O_FILES := $(patsubst %.$(SRC_SUFFIX),.build/%.o,$(wildcard myAnalysis/*.$(SRC_SUFFIX)))
MAIN_O_FILES    := $(patsubst %.$(SRC_SUFFIX),.build/%.o,$(wildcard src/*.$(SRC_SUFFIX)))
EXE_O_FILES     := $(UTIL_O_FILES) $(SANDBOX_O_FILES) $(SCRIPT_O_FILES) $(ANALYSIS_O_FILES) $(PROOF_O_FILES)
EXECUTABLES     := $(patsubst %.o,bin/%,$(notdir $(EXE_O_FILES))) bin/grsisort

HISTOGRAM_SO    := $(patsubst histos/%.$(SRC_SUFFIX),lib/lib%.so,$(wildcard histos/*.$(SRC_SUFFIX)))
FILTER_SO    := $(patsubst filters/%.$(SRC_SUFFIX),lib/lib%.so,$(wildcard filters/*.$(SRC_SUFFIX)))

PARSER_LIBRARIES := $(shell ls -d GRSIData ILLData iThembaData HILData 2> /dev/null)

ifdef VERBOSE
run_and_test = @echo $(1) && $(1);
else
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
endif

all: include/GVersion.h grsirc $(EXECUTABLES) $(LIBRARY_OUTPUT) lib/libGRSI.so config $(HISTOGRAM_SO) $(FILTER_SO)
	@$(FIND) .build users -name "*.pcm" -exec cp {} lib/ \;
	@$(FIND) .build users -name "*.rootmap" -exec cp {} lib/ \;
	@$(FIND) . -maxdepth 1 -name "*.pcm" -exec mv {} lib/ \;
	@printf "$(OK_COLOR)Compilation successful, $(WARN_COLOR)woohoo!$(NO_COLOR)\n"

docs: doxygen

doxygen:
	$(MAKE) -C $@

bin/grsisort: $(MAIN_O_FILES) | $(LIBRARY_OUTPUT) bin include/GVersion.h
	$(call run_and_test,$(CPP) $^ -o $@ $(LINKFLAGS),$@,$(COM_COLOR),$(COM_STRING),$(OBJ_COLOR) )

bin/%: .build/util/%.o | $(LIBRARY_OUTPUT) bin include/GVersion.h
	$(call run_and_test,$(CPP) $< -o $@ $(LINKFLAGS),$@,$(COM_COLOR),$(COM_STRING),$(OBJ_COLOR) )

bin/%: .build/Sandbox/%.o | $(LIBRARY_OUTPUT) bin include/GVersion.h
	$(call run_and_test,$(CPP) $< -o $@ $(LINKFLAGS),$@,$(COM_COLOR),$(COM_STRING),$(OBJ_COLOR) )

bin/%: .build/scripts/%.o | $(LIBRARY_OUTPUT) bin include/GVersion.h
	$(call run_and_test,$(CPP) $< -o $@ $(LINKFLAGS),$@,$(COM_COLOR),$(COM_STRING),$(OBJ_COLOR) )

bin/%: .build/GRSIProof/%.o | $(LIBRARY_OUTPUT) bin include/GVersion.h
	$(call run_and_test,$(CPP) $< -o $@ $(LINKFLAGS),$@,$(COM_COLOR),$(COM_STRING),$(OBJ_COLOR) )

bin/%: .build/myAnalysis/%.o | $(LIBRARY_OUTPUT) bin include/GVersion.h
	$(call run_and_test,$(CPP) $< -o $@ $(LINKFLAGS),$@,$(COM_COLOR),$(COM_STRING),$(OBJ_COLOR) )

bin lib:
	@mkdir -p $@

include/GVersion.h: 
	$(call run_and_test,util/gen_version.sh,$@,$(COM_COLOR),$(COM_STRING),$(OBJ_COLOR) )

#include/GVersion.h: .git/HEAD .git/index util/gen_version.sh

grsirc:
	$(call run_and_test,util/gen_grsirc.sh,$@,$(COM_COLOR),$(BLD_STRING),$(OBJ_COLOR) )

lib/lib%.so: .build/histos/%.o | include/GVersion.h lib
	$(call run_and_test,$(CPP) -fPIC $^ $(SHAREDSWITCH)lib$*.so $(ROOT_LIBFLAGS) -o $@,$@,$(BLD_COLOR),$(BLD_STRING),$(OBJ_COLOR) )

lib/lib%.so: .build/filters/%.o | include/GVersion.h lib
	$(call run_and_test,$(CPP) -fPIC $^ $(SHAREDSWITCH)lib$*.so $(ROOT_LIBFLAGS) -o $@,$@,$(BLD_COLOR),$(BLD_STRING),$(OBJ_COLOR) )

config: bin
	@cp util/grsi-config bin/
	@cp util/ErrorReport.sh bin/

# Functions for determining the files included in a library.
# All src files in the library directory are included.
# If a LinkDef.h file is present in the library directory,
#    a dictionary file will also be generated and added to the library.
libdir          = $(shell $(FIND) libraries -name $(1) -type d)
lib_src_files   = $(shell $(FIND) $(call libdir,$(1)) -name "*.$(SRC_SUFFIX)")
lib_o_files     = $(patsubst %.$(SRC_SUFFIX),.build/%.o,$(call lib_src_files,$(1)))
lib_linkdef     = $(wildcard $(call libdir,$(1))/LinkDef.h)
lib_dictionary  = $(patsubst %/LinkDef.h,.build/%/LibDictionary.o,$(call lib_linkdef,$(1)))

lib/lib%.so: $$(call lib_o_files,%) $$(call lib_dictionary,%) | lib include/GVersion.h
	$(call run_and_test,$(CPP) -fPIC $^ $(SHAREDSWITCH)lib$*.so $(ROOT_LIBFLAGS) -o $@,$@,$(BLD_COLOR),$(BLD_STRING),$(OBJ_COLOR) )

lib/libGRSI.so: $(LIBRARY_OUTPUT) | include/GVersion.h
	$(call run_and_test,$(CPP) -fPIC $(shell $(FIND) .build/libraries -name "*.o") $(SHAREDSWITCH)lib$*.so $(ROOT_LIBFLAGS) -o $@,$@,$(BLD_COLOR),$(BLD_STRING),$(OBJ_COLOR) )

.build/%.o: %.$(SRC_SUFFIX) | include/GVersion.h
	@mkdir -p $(dir $@)
	$(call run_and_test,$(CPP) -fPIC -c $< -o $@ $(CFLAGS),$@,$(COM_COLOR),$(COM_STRING),$(OBJ_COLOR) )

dict_header_files = $(addprefix $(PWD)/include/,$(subst //,,$(shell $(HEAD) $(1) -n 1 2> /dev/null)))
find_linkdef = $(shell $(FIND) $(1) -name "*LinkDef.h")

# In order for all function names to be unique, rootcint requires unique output names.
# Therefore, usual wildcard rules are insufficient.
# Eval is more powerful, but is less convenient to use.
define library_template
.build/$(1)/$(notdir $(1))Dict.cxx: $(1)/LinkDef.h $$(call dict_header_files,$(1)/LinkDef.h) 
	@mkdir -p $$(dir $$@)
	$$(call run_and_test,$$(ROOTCINT) -f $$@ $$(INCLUDES) -I/opt/local/include $$(RCFLAGS) -s $(notdir $(1)) -multiDict -rml lib$(notdir $(1)).so -rmf .build/$(1)/$(notdir $(1)).rootmap $$(notdir $$(filter-out $$<,$$^)) $$<,$$@,$$(COM_COLOR),$$(BLD_STRING) ,$$(OBJ_COLOR))

.build/$(1)/LibDictionary.o: .build/$(1)/$(notdir $(1))Dict.cxx
	$$(call run_and_test,$$(CPP) -fPIC -c $$< -o $$@ $$(CFLAGS),$$@,$$(COM_COLOR),$$(COM_STRING),$$(OBJ_COLOR) )
endef

$(foreach lib,$(LIBRARY_DIRS),$(eval $(call library_template,$(lib))))

-include $(shell $(FIND) .build -name '*.d' 2> /dev/null)

html: all
	@printf " ${COM_COLOR}Building      ${OBJ_COLOR} HTML Documentation ${NO_COLOR}\n"
	@cp -r include grsisort
	@grsisort -q -l --work_harder util/html_generator.C #>/dev/null
	@$(RM) -r grsisort
	@$(RM) tempfile.out

complete: all parsers

parsers: all
	@$(foreach parser,$(PARSER_LIBRARIES),$(MAKE) -C $(parser);)

GRSIData: all
	@$(MAKE) -C GRSIData

ILLData: all
	@$(MAKE) -C ILLData

iThembaData: all
	@$(MAKE) -C iThembaData

HILData: all
	@$(MAKE) -C HILData

clean:
	@printf "\n$(WARN_COLOR)Cleaning up$(NO_COLOR)\n\n"
	@-$(RM) -rf .build bin lib include/GVersion.h
	@-$(RM) -rf libraries/*.so libraries/*.pcm #this is here for cleaning up libraries from pre GRSI 3.0

cleaner: clean
	@printf "\nEven more clean up\n\n"
	@-$(RM) -rf htmldoc
