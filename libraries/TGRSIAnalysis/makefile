<<<<<<< HEAD
SUBDIRS = TGRSIDetector TGriffin TSceptar TPaces#TDescant #TTip TTigress  TSharc TTriFoil TCSM 
=======
SUBDIRS = TGRSIDetector TGriffin TSceptar TDescant TPaces TTigress #TTip  TSharc TTriFoil TCSM 
>>>>>>> 61a344efa789327adb9cdaa29dae2817bbc1c3a6
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


