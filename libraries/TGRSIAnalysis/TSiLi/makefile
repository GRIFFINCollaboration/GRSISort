
OBJECTS = TSiLi.o TSiLiData.o TSiLiHit.o TSiLiDict.o 
INCDIR = $(BASE)/include
DICTOBJ = $(INCDIR)/TSiLiHit.h $(INCDIR)/TSiLi.h
DICTNAM = TSiLi

CFLAGS += -fPIC

#COMP_STRING="Now Compling "
DICT_STRING="Now Making Dict for ${OBJ_COLOR}$< ${NO_COLOR}"

CAT=cat

.PHONY: all clean gone



all: libSiLi.so
	@printf "\r ${FIN_COLOR}%s${FIN_OBJ_COLOR}%-30s ${NO_COLOR}\n" $(FIN_STRING) $^ ;


libSiLi.so: $(OBJECTS)
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
	@cp $@ ../..


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
	@$(RM) -f temp.errors temp.log


%Dict.cxx: $(BASE)/include/%.h
	@printf " ${COM_COLOR}%s${DICT_COLOR}%s${NO_COLOR}" $(COMP_STRING) $@
	@rootcint -f $@ -c $^ 2> temp.log || touch temp.errors
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

$(DICTNAM)Dict.cxx: $(DICTOBJ)
	@printf " ${COM_COLOR}%s${DICT_COLOR}%s${NO_COLOR}" $(COMP_STRING) $@
	@rootcint -f $@ -c -I$(INCDIR) -p $(DICTOBJ) $(DICTNAM)LinkDef.h 2> temp.log || touch temp.errors
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







clean:
	$(RM) $(OBJECTS) *~ *Dict* *so 



