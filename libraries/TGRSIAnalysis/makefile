SUBDIRS = TGRSIDetector TTip TTigress TGriffin TSharc TTriFoil TRF TPaces TDescant TSceptar TCSM TNucleus TKinematics TReaction TSRIM TBetaDecay TAnalysisTreeBuilder TCal TGRSIFit
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


