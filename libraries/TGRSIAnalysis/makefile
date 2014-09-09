SUBDIRS = TTigress TSharc TNucleus TKinematics TAnalysisTreeBuilder
ALLDIRS = $(SUBDIRS)

.PHONY: subdirs $(ALLDIRS) clean
subdirs: $(SUBDIRS)

$(ALLDIRS):
	@$(MAKE) -C $@

clean:
	@$(RM) *a *so *~ .nsf*
	@for dir in $(ALLDIRS); do \
		$(MAKE) -C $$dir $@; \
	done


