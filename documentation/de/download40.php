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

<h3>Alpha-/Entwicklerversionen</h3>

<b>Helfen Sie beim Testen!</b> Hier finden Sie experimentelle 
dvdisaster-Versionen, die auf dem Weg zur nächsten "stabilen" Version
entstehen.<p>

<b>Ein Wort der Vorsicht:</b> Alpha-Versionen sind nicht so intensiv getestet
wie die stabilen Versionen. Sie können mehr Fehler enthalten und 
sollten nicht zum Bearbeiten von wichtigen Daten
verwendet werden.<p>

Verwenden Sie im Zweifelsfall die <a href="download.php">stabile Version 0.72</a>
und warten Sie auf die Veröffentlichung von Version 0.80.

<hr>

<h3>Herunterladen</h3>

Einen aktuellen Überblick über verfügbare Alpha-Versionen finden
Sie in der <a href="http://dvdisaster.net/de/download40.php">Online-Version dieser Seiten</a>.

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
