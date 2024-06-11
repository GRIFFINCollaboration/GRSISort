#!/bin/bash

# disable check for undefined variables, otherwise it would complain about DOXYGEN_GRSISORT_VERSION which is set outside this script
# shellcheck disable=SC2154

echo "<!-- HTML footer for doxygen 1.9.8-->"
echo "<!-- start footer part -->"
echo "<!--BEGIN GENERATE_TREEVIEW-->"
echo "<div id=\"nav-path\" class=\"navpath\"><!-- id is needed for treeview function! -->"
echo "  <ul>"
echo "    \$navpath"
echo "    <li class=\"footer\">GRSISort ($DOXYGEN_GRSISORT_VERSION) Reference Guide Generated on \$datetime.</li>"
echo "  </ul>"
echo "</div>"
echo "<!--END GENERATE_TREEVIEW-->"
echo "<!--BEGIN !GENERATE_TREEVIEW-->"
echo "<hr class=\"footer\"/><address class=\"footer\"><small>"
echo "\$generatedby&#160;<a href=\"https://www.doxygen.org/index.html\"><img class=\"footer\" src=\"\$relpath^doxygen.svg\" width=\"104\" height=\"31\" alt=\"doxygen\"/></a> \$doxygenversion"
echo "</small></address>"
echo "<!--END !GENERATE_TREEVIEW-->"
echo "</body>"
echo "</html>"
