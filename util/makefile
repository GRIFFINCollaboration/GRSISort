CXXTARGET = Root2Rad
ROOTTARGET = MakeMatrices MakeTimeDiffSpec Make1DChargeHistograms offsetadd offsetfind



CFLAGS += -fPIC
CFLAGS += `root-config --cflags --libs`

BINDIR = $(GRSISYS)/bin

#COMP_STRING="Now Compling "
DICT_STRING="Now Making Dict for ${OBJ_COLOR}$< ${NO_COLOR}"

CAT=cat

.PHONY: all clean gone end

UTILFLAGS =  -L$(GRSISYS)/libraries  -lAnalysisTreeBuilder -lGriffin -lSceptar -lDescant -lPaces -lGRSIDetector -lTigress -lSharc -lCSM -lTriFoil -lTGRSIint -lGRSILoop -lMidasFormat -lGRSIRootIO -lDataParser -lGRSIFormat -lMidasFormat -lXMLParser -lXMLIO -lProof -lGuiHtml `root-config --cflags --libs` -lTreePlayer -lGROOT -lX11 -lXpm


UTIL_STRING = "Finished compiling utilities"

all: $(addprefix $(BINDIR)/,$(CXXTARGET) $(ROOTTARGET)) end

end:
	@printf "\r ${FIN_COLOR}%s${FIN_OBJ_COLOR}%-30s ${NO_COLOR}\n" $(UTIL_STRING) $^ ;

$(BINDIR)/%: %.cxx
	@printf " ${COM_COLOR}%s ${OBJ_COLOR}%s${NO_COLOR}" $(COMP_STRING) $(basename $^) 
	@$(CXX) $^ $(CFLAGS) $(CPPFLAGS) -o$@ 2> temp.log || touch temp.errors
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
#@mv $@ $(GRSISYS)/bin/$@

$(BINDIR)/%: %.C
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
	$(RM) $(GRSISYS)/bin/$(ROOTTARGET) $(GRSISYS)/bin/$(CXXTARGET) *~ *Dict* *so 

