# Generates the HTML footer

echo '<html>'
echo '</body>'
echo '<div id="footer" style="background-color:#DDDDDD;">'
echo '<small>'
#echo '<img class="footer" src="grsisortbanner.png" alt="root"/></a>'
echo 'GRSISort ('$DOXYGEN_GRSISORT_VERSION') Reference Guide Generated on $datetime.'
echo '</small>'
echo '</div>'
echo '</body>'
echo '</html>'
