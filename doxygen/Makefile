
.PHONY: filter doxygen clean

export DOXYGEN_GRSISORT_VERSION=$(shell grsi-config --version)
export DOXYGEN_OUTPUT_DIRECTORY=$(GRSISYS)/documentation
export DOXYGEN_EXAMPLE_PATH=$(DOXYGEN_OUTPUT_DIRECTORY)/macros

all: doxygen

filter:
	`root-config --cxx` -o filter filter.cxx `root-config --libs --glibs --cflags`

doxygen:
	if [ ! -d $(DOXYGEN_OUTPUT_DIRECTORY) ]; then mkdir $(DOXYGEN_OUTPUT_DIRECTORY); fi
	if [ ! -d $(DOXYGEN_EXAMPLE_PATH) ]; then mkdir $(DOXYGEN_EXAMPLE_PATH); fi
	if [ ! -d $(DOXYGEN_IMAGE_PATH) ]; then mkdir $(DOXYGEN_IMAGE_PATH); fi
	./makehtmlfooter.sh > htmlfooter.html
	doxygen
	rm -f c1*
	rm -f stdout.dat
	rm -f htmlfooter.html

clean:
	rm -rf $(DOXYGEN_OUTPUT_DIRECTORY)
	rm -f filter
