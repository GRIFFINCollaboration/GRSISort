SUBDIRS = TGRSIDetector TGriffin #TTip TTigress  TSharc TTriFoil TPaces TDescant TSceptar TCSM 
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


