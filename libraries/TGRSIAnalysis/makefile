SUBDIRS = TGRSIDetector TTigress TGriffin TSharc TTriFoil TSceptar TCSM TNucleus TKinematics TSRIM TBetaDecay  TAnalysisTreeBuilder TECal TGRSIFit 
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


