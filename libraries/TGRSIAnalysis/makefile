SUBDIRS = TGRSIDetector TGriffin TSceptar TPaces#TDescant #TTip TTigress  TSharc TTriFoil TCSM 
SUBDIRS += TNucleus TKinematics TSRIM TBetaDecay TAnalysisTreeBuilder TCal TGRSIFit
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


