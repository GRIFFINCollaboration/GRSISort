SUBDIRS = TGRSIDetector TBetaDecay TAnalysisTreeBuilder TCal TGRSIFit TNucleus TKinematics TGriffin TSRIM TTigress TSceptar #TSharc TTriFoil TPaces TDescant TCSM TTip
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


