<?php
# dvdisaster: German homepage translation
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

<h3 class="top">Hintergrundinformationen</h3>

Die Informationen in diesem Abschnitt werden nicht unbedingt für die
Bedienung von dvdisaster benötigt. Sie sind aber hilfreich um zu verstehen
wie dvdisaster funktioniert und können Ihnen dabei helfen, das Programm
entsprechend Ihren Bedürfnissen anzuwenden.

<ol>
<li><a href="qa31.php">Eigenschaften der Reed-Solomon-Fehlerkorrektur</a><p></li>
<li><a href="qa32.php">Datenrekonstruktion auf Abbild-Ebene</a><p></li>
<li><a href="qa33.php">Die RS01, RS02 und RS03-Verfahren</a><p></li>
<li><a href="qa34.php">Arbeitsweise des linearen Lese-Verfahrens</a><p></li>   
<li><a href="qa35.php">Arbeitsweise des angepaßten Lese-Verfahrens</a><p></li>   
<li><a href="qa36.php">Einige Bemerkungen zu Lesefehlern</a><p></li>   
<li><a href="qa37.php">Tips zum Aufbewahren der Fehlerkorrektur-Datei</a><p></li>
</ol>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
