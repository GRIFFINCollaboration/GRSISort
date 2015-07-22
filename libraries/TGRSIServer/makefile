
OBJECTS = TGRSIServer.o TGRSIServerDict.o 
NAME = TGRSIServer

-include ../../make.rules

%Dict.cxx: $(INCDIR)/%.h
	@printf " ${COM_COLOR}%s${DICT_COLOR}%s${NO_COLOR}" $(COMP_STRING) $@
	@rootcint -f $@ -c -p -DMAJOR_ROOT_VERSION=${MAJOR_ROOT_VERSION} $^ 2> temp.log || touch temp.errors
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
