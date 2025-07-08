#!/bin/bash

# run clang-format using our .clang-format settings file over all header and source files in the include, libraries, util, examples, GRSIProof, and src directories

find include/ libraries/ util/ examples/ GRSIProof/ src/ \( -name "*.h" -o -name "*.hh" -o -name "*.cxx" -o -name "*.C" \) -exec echo "processing file {}" \; -exec clang-format -style=file -i {} \;

