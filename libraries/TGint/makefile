
OBJECTS = TGint.o TGintDict.o 

CFLAGS += -fPIC

COMP_STRING=-n "\033[0;34m Now Compling \033[0;36m$< \033[0m"
DICT_STRING=-n "\033[0;34m Now Make Dict for \033[0;36m$< \033[0m"

CAT=cat

.PHONY: all clean gone

all: libTGint.so

libTGint.so: $(OBJECTS)
	@$(ECHO) $(COMP_STRING)
	@$(COMPILESHARED)$@ $(CFLAGS) -olibTGint.so $(OBJECTS) words.o  2> temp.log || touch temp.errors
	@if test -e temp.errors; then $(ECHO) $(ERROR_STRING) && $(CAT) temp.log && \
	 $(ECHO) "\033[0;31m from $^ in ${PWD} \033[m"; \
	 elif test -s temp.log; then $(ECHO) $(WARN_STRING) && $(CAT) temp.log; \
	 else $(ECHO) $(OK_STRING); \
	 fi;
	@$(RM) -f temp.errors temp.log
	@cp libTGint.so ..



%.o: %.cxx
	@$(ECHO) $(COMP_STRING)
	@$(CXX) -c $^ $(CFLAGS) $(CPPFLAGS) 2> temp.log || touch temp.errors
	@if test -e temp.errors; then $(ECHO) $(ERROR_STRING) && $(CAT) temp.log && \
	 $(ECHO) "\033[0;31m from $^ in ${PWD} \033[m"; \
	 elif test -s temp.log; then $(ECHO) $(WARN_STRING) && $(CAT) temp.log; \
	 else $(ECHO) $(OK_STRING); \
	 fi;
	@$(RM) -f temp.errors temp.log

%Dict.cxx: $(BASE)/include/%.h
	@$(ECHO) $(DICT_STRING)
	@rootcint $@ -c $^ 2> temp.log || touch temp.errors
	@if test -e temp.errors; then $(ECHO) $(ERROR_STRING) && $(CAT) temp.log && \
		$(ECHO) "\033[0;31m from $^ in ${PWD} \033[m"; \
		elif test -s temp.log; then $(ECHO) $(WARN_STRING) && $(CAT) temp.log; \
		else $(ECHO) $(OK_STRING); \
		fi;
	@$(RM) -f temp.errors temp.log



clean:
	$(RM) $(OBJECTS) *~ *Dict* 



