CXXTARGET = Root2Rad
ROOTTARGET = MakeMatrices MakeTimeDiffSpec

CFLAGS += -fPIC
CFLAGS += `root-config --cflags --libs`

BINDIR = $(GRSISYS)/bin

#COMP_STRING="Now Compling "
DICT_STRING="Now Making Dict for ${OBJ_COLOR}$< ${NO_COLOR}"

CAT=cat

.PHONY: all clean gone end

UTILFLAGS = -L$(GRSISYS)/libraries -lGRSIFormat -lAnalysisTreeBuilder -lGriffin -lSceptar -lDescant -lPaces -lGRSIDetector -lTigress -lSharc -lCSM -lTriFoil -lTGRSIint -lGRSILoop -lMidasFormat -lGRSIRootIO -lDataParser -lMidasFormat -lXMLParser -lXMLIO -lProof -lGuiHtml `root-config --cflags --libs` -lTreePlayer -lX11 -lXpm


UTIL_STRING = "Finished compiling utilities"

all: $(CXXTARGET) $(ROOTTARGET) end

end:
	@printf "\r ${FIN_COLOR}%s${FIN_OBJ_COLOR}%-30s ${NO_COLOR}\n" $(UTIL_STRING) $^ ;

%: %.cxx
	@printf " ${COM_COLOR}%s ${OBJ_COLOR}%s${NO_COLOR}" $(COMP_STRING) $@ 
	@$(CXX) $^ $(CFLAGS) $(CPPFLAGS) -o$(BINDIR)/$@ 2> temp.log || touch temp.errors
	@if test -e temp.errors; then \
		printf "\r ${COM_COLOR}%s${OBJ_COLOR}%-30s ${ERROR_COLOR}%*s${NO_COLOR}\n" $(COMP_STRING) $@ 10 $(ERROR_STRING) \
		&& $(CAT) temp.log && \
		printf "${ERROR_COLOR}%s\n${NO_COLOR}" ${PWD};  \
		elif test -s temp.log; then \
		printf "\r ${COM_COLOR}%s${OBJ_COLOR}%-30s ${WARN_COLOR}%*s${NO_COLOR}\n" $(COMP_STRING) $@ 10 $(WARN_STRING) \
		&& $(CAT) temp.log; \
		else printf "\r ${COM_COLOR}%s${OBJ_COLOR}%-30s ${OK_COLOR}%*s${NO_COLOR}\n" $(COMP_STRING) $@ 10  $(OK_STRING); \
		fi;
	@$(RM) -f temp.errors temp.log temp.comp
#@mv $@ $(GRSISYS)/bin/$@

%: %.C
	@printf " ${COM_COLOR}%s ${OBJ_COLOR}%s${NO_COLOR}" $(COMP_STRING) $@ 
	@$(CXX) $^ $(CFLAGS) $(CPPFLAGS) $(UTILFLAGS) -o$(BINDIR)/$@ 2> temp.log || touch temp.errors
	@if test -e temp.errors; then \
		printf "\r ${COM_COLOR}%s${OBJ_COLOR}%-30s ${ERROR_COLOR}%*s${NO_COLOR}\n" $(COMP_STRING) $@ 10 $(ERROR_STRING) \
		&& $(CAT) temp.log && \
		printf "${ERROR_COLOR}%s\n${NO_COLOR}" ${PWD};  \
		elif test -s temp.log; then \
		printf "\r ${COM_COLOR}%s${OBJ_COLOR}%-30s ${WARN_COLOR}%*s${NO_COLOR}\n" $(COMP_STRING) $@ 10 $(WARN_STRING) \
		&& $(CAT) temp.log; \
		else printf "\r ${COM_COLOR}%s${OBJ_COLOR}%-30s ${OK_COLOR}%*s${NO_COLOR}\n" $(COMP_STRING) $@ 10  $(OK_STRING); \
		fi;
	@$(RM) -f temp.errors temp.log temp.comp

clean:
	$(RM) $(GRSISYS)/bin/$(ROOTTARGET) $(GRSISYS)/bin/$(CXXTARGET) *~ *Dict* *so 

