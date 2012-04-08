<?php
# dvdisaster: English homepage translation
# Copyright (C) 2004-2012 Carsten Gnörlich
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

require("../include/dvdisaster.php");
begin_page();
?>

<!-- Insert actual page content below -->

<h3 class="top">Background information</h3>

The information in this sub section is not required for operating dvdisaster.
However it is helpful on understanding how dvdisaster works and may help
you getting the most out of the program according to your needs.

<ol>
<li><a href="qa31.php">Properties of the Reed-Solomon error correction</a><p></li>
<li><a href="qa32.php">Image level data recovery</a><p></li>
<li><a href="qa33.php">The RS01, RS02 and RS03 methods</a><p></li>
<li><a href="qa34.php">Details of the linear reading strategy</a><p></li>   
<li><a href="qa35.php">Details of the adaptive reading strategy</a><p></li>   
<li><a href="qa36.php">Some remarks on read errors</a><p></li>   
<li><a href="qa37.php">Hints for storing the error correction files</a><p></li>
</ol>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
