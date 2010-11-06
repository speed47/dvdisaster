<?php
# dvdisaster: German homepage translation
# Copyright (C) 2004-2010 Carsten Gnörlich
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

require("../include/dvdisaster.php");
begin_page();
?>

<!--- Insert actual page content below --->

<h3>Additional resources</h3>

The online documentation which you are currently reading is included
in the dvdisaster program packages. You do not need to download
it separately.<p>

The following additional papers are available:<p>

<b>RS03 specification</b><p>

RS03 is a new encoding format for upcoming dvdisaster versions,
capable of using multiple processor cores for its
calculations. This can not be done with the current RS01
und RS02 methods due to limits in their internal structure.<p>

A <a href="http://dvdisaster.net/papers/rs03.pdf">preview of the RS03 specification (rs03.pdf)</a>
is available now for discussion. The specification is not final.<p>

Reading the RS03 document requires knowledge in the area of
coding theory. It is not meant as end user documentation.

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
