PLATFORM = $(shell uname)

export PLATFORM:= $(PLATFORM)

ifeq ($(PLATFORM),Darwin)
export __APPLE__:= 1
export CFLAGS += -DOS_DARWIN -DHAVE_ZLIB #-lz
export CFLAGS += -I/opt/X11/include -Qunused-arguments
export LFLAGS = -dynamiclib -undefined dynamic_lookup -single_module # 
export SHAREDSWITCH = -install_name # ENDING SPACE
export CPP = clang++ 
export CXX = clang++
else
export __LINUX__:= 1	
export CFLAGS += -Wl,--no-as-needed 
export SHAREDSWITCH = -shared -Wl,-soname,#NO ENDING SPACE
export CPP = g++
endif

LIBDIR = -L$(BASE)/libraries
ifeq ($(PLATFORM),Darwin)
LIBDIR += -L/opt/X11/lib 
endif
ifdef MIDASSYS
ifeq ($(PLATFORM),Darwin)
LIBDIR += -L$(MIDASSYS)/darwin/lib
else ifeq ($(PLATFORM),Linux)
LIBDIR += -L$(MIDASSYS)/linux/lib
endif
endif

ifndef __APPLE__
LFLAGS = -Wl,--no-as-needed
endif

CFLAGS += -fPIC -O2
CFLAGS += -std=c++0x -O2 -I$(PWD)/include -g `root-config --cflags --libs`

CXXTARGET=exAnalysis

BINDIR = $(GRSISYS)/bin

#COMP_STRING="Now Compling "
DICT_STRING="Now Making Dict for ${OBJ_COLOR}$< ${NO_COLOR}"

CAT=cat

.PHONY: all clean gone end

#UTILFLAGS = `grsi-config --cflags --all-libs --root`
UTILFLAGS = -lGriffin -lSceptar -lDescant -lPaces -lGRSIDetector -lTigress

UTILFLAGS += $(LIBDIR) -lAnalysisTreeBuilder -lTGRSIFit -lTGRSIint -lGRSILoop -lMidasFormat -lGRSIRootIO -lDataParser -lGRSIFormat -lMidasFormat -lXMLParser -lXMLIO -lProof -lGuiHtml -lTreePlayer -lGROOT -lX11 -lXpm -lSpectrum


UTIL_STRING = "Finished compiling examples"


all: end

end:$(addprefix $(BINDIR)/,$(CXXTARGET))
	@printf "\r ${FIN_COLOR}%s${FIN_OBJ_COLOR}%-30s ${NO_COLOR}\n" $(UTIL_STRING);

$(BINDIR)/%: %.cxx
	@printf " ${COM_COLOR}%s ${OBJ_COLOR}%s${NO_COLOR}" $(COMP_STRING) $(basename $^) 
	@$(CXX) $^ $(CFLAGS) $(CPPFLAGS) $(UTILFLAGS) -o$@ 2> temp.log || touch temp.errors
	@if test -e temp.errors; then \
		printf "\r ${COM_COLOR}%s${OBJ_COLOR}%-30s ${ERROR_COLOR}%*s${NO_COLOR}\n" $(COMP_STRING) $(basename $^) 10 $(ERROR_STRING) \
		&& $(CAT) temp.log && \
		printf "${ERROR_COLOR}%s\n${NO_COLOR}" ${PWD};  \
		elif test -s temp.log; then \
		printf "\r ${COM_COLOR}%s${OBJ_COLOR}%-30s ${WARN_COLOR}%*s${NO_COLOR}\n" $(COMP_STRING) $(basename $^) 10 $(WARN_STRING) \
		&& $(CAT) temp.log; \
		else printf "\r ${COM_COLOR}%s${OBJ_COLOR}%-30s ${OK_COLOR}%*s${NO_COLOR}\n" $(COMP_STRING) $(basename $^) 10  $(OK_STRING); \
		fi;
	@$(RM) -f temp.errors temp.log temp.comp

clean:
	$(RM) $(GRSISYS)/bin/$(CXXTARGET) *~ *Dict* *so 

