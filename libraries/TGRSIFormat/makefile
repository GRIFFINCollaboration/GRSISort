
#OBJECTS = TFragment.o TFragmentDict.o TChannel.o TChannelDict.o TFragmentQueue.o TFragmentQueueDict.o TGRSIStats.o TGRSIStatsDict.o TFragmentSelector.o TFragmentSelectorDict.o TGRSIRunInfo.o TGRSIRunInfoDict.o 

OBJECTS = TFragment.o TChannel.o TFragmentQueue.o TGRSIStats.o TFragmentSelector.o TGRSIRunInfo.o TEpicsFrag.o TGRSISortInfo.o TGRSIFormatDict.o 

CFLAGS += -fPIC
CFLAGS += -I $(BASE)/users

#COMP_STRING="Now Compling "
DICT_STRING="Now Making Dict for ${OBJ_COLOR}$< ${NO_COLOR}"

INCDIR = $(BASE)/include

CAT=cat

.PHONY: all clean gone

all: libGRSIFormat.so $(OBJECTS)
	@printf "\r ${FIN_COLOR}%s${FIN_OBJ_COLOR}%-30s ${NO_COLOR}\n" $(FIN_STRING) $< ;

USERDEPS = $(BASE)/users/UserInitObj.h $(BASE)/users/UserFillObj.h

INCLUDES = UserInitObj.h UserFillObj.h

libGRSIFormat.so: $(OBJECTS)
	@printf " ${COM_COLOR}%s${OBJ_COLOR}%s${NO_COLOR}" $(COMP_STRING) "$@"
	@$(COMPILESHARED)$@ $(CFLAGS) -o$@ $(OBJECTS) 2> temp.log || touch temp.errors
	@if test -e temp.errors; then \
		printf "\r ${COM_COLOR}%s${OBJ_COLOR}%-30s ${ERROR_COLOR}%*s${NO_COLOR}\n" $(COMP_STRING) $@ 10 $(ERROR_STRING) \
		&& $(CAT) temp.log && \
		printf "${ERROR_COLOR}%s\n${NO_COLOR}" ${PWD};  \
		elif test -s temp.log; then \
		printf "\r ${COM_COLOR}%s${OBJ_COLOR}%-30s ${WARN_COLOR}%*s${NO_COLOR}\n" $(COMP_STRING) $@ 10 $(WARN_STRING) \
		&& $(CAT) temp.log; \
		else printf "\r ${COM_COLOR}%s${OBJ_COLOR}%-30s ${OK_COLOR}%*s${NO_COLOR}\n" $(COMP_STRING) $@ 10  $(OK_STRING); \
		fi;
	@$(RM) -f temp.errors temp.log
	@cp $@ ..

%.o: %.cxx
	@printf " ${COM_COLOR}%s ${OBJ_COLOR}%s${NO_COLOR}" $(COMP_STRING) $@ 
	@$(CXX) -c $^ $(CFLAGS) $(CPPFLAGS) 2> temp.log || touch temp.errors
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



#%Dict.cxx: $(BASE)/include/%.h
#	@printf " ${COM_COLOR}%s${DICT_COLOR}%s${NO_COLOR}" $(COMP_STRING) $@
#	@rootcint -f $@ -c $^ 2> temp.log || touch temp.errors
#	@if test -e temp.errors; then \
		printf "\r ${COM_COLOR}%s${DICT_COLOR}%-30s ${ERROR_COLOR}%*s${NO_COLOR}\n" $(COMP_STRING) $@ 10 $(ERROR_STRING) \
		&& $(CAT) temp.log && \
		printf "${ERROR_COLOR}%s\n${NO_COLOR}" ${PWD};  \
		elif test -s temp.log; then \
		printf "\r ${COM_COLOR}%s${DICT_COLOR}%-30s ${WARN_COLOR}%*s${NO_COLOR}\n" $(COMP_STRING) $@ 10 $(WARN_STRING) \
		&& $(CAT) temp.log; \
		else printf "\r ${COM_COLOR}%s${DICT_COLOR}%-30s ${OK_COLOR}%*s${NO_COLOR}\n" $(COMP_STRING) $@ 10  $(OK_STRING); \
		fi;
#	@$(RM) -f temp.errors temp.log

TGRSIFormatDict.cxx: $(INCDIR)/TFragment.h $(INCDIR)/TChannel.h $(INCDIR)/TFragmentQueue.h $(INCDIR)/TFragmentSelector.h $(INCDIR)/TGRSIRunInfo.h $(INCDIR)/TGRSIStats.h $(INCDIR)/TEpicsFrag.h $(INCDIR)/TGRSISortInfo.h
	@printf " ${COM_COLOR}%s${DICT_COLOR}%s${NO_COLOR}" $(COMP_STRING) $@
	@rootcint -f $@ -c -p $(INCDIR)/TFragment.h $(INCDIR)/TChannel.h $(INCDIR)/TFragmentQueue.h $(INCDIR)/TFragmentSelector.h $(INCDIR)/TGRSIRunInfo.h $(INCDIR)/TGRSIStats.h $(INCDIR)/TEpicsFrag.h $(INCDIR)/TGRSISortInfo.h TGRSIFormatLinkDef.h 2> temp.log || touch temp.errors
	@if test -e temp.errors; then \
		printf "\r ${COM_COLOR}%s${DICT_COLOR}%-30s ${ERROR_COLOR}%*s${NO_COLOR}\n" $(COMP_STRING) $@ 10 $(ERROR_STRING) \
		&& $(CAT) temp.log && \
		printf "${ERROR_COLOR}%s\n${NO_COLOR}" ${PWD};  \
		elif test -s temp.log; then \
		printf "\r ${COM_COLOR}%s${DICT_COLOR}%-30s ${WARN_COLOR}%*s${NO_COLOR}\n" $(COMP_STRING) $@ 10 $(WARN_STRING) \
		&& $(CAT) temp.log; \
		else printf "\r ${COM_COLOR}%s${DICT_COLOR}%-30s ${OK_COLOR}%*s${NO_COLOR}\n" $(COMP_STRING) $@ 10  $(OK_STRING); \
		fi;
	@$(RM) -f temp.errors temp.log


#TGRSIFormatDict.o:  TGRSIFormatDict.cxx
#	$(CXX) -c $^ $(CFLAGS) $(CPPFLAGS) 


clean:
	$(RM) $(OBJECTS) *~ *Dict* *so  











