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

<h3 class="top">Das dvdisaster-Projekt</h3>


Optische Datenträger (also CD,DVD,BD) behalten ihre Daten nur 
eine endlich lange Zeit (normalerweise
viele Jahre). Danach beginnt typischerweise ein langsamer Datenverlust, 
indem von außen nach innen immer mehr Lesefehler auftreten.<p>

<b>Archivieren mit Schutz vor Datenverlust</b><p>

dvdisaster archiviert Daten so auf CD/DVD/BD (<a href="qa10.php#media">unterstützte Formate</a>), 
daß sie auch dann noch wiederherstellbar sind, 
wenn der Datenträger bereits einige Lesefehler enthält. Dadurch
können Sie Ihre Daten noch vollständig auf einen neuen Datenträger retten.<p>

Der Schutz vor Datenverlust geschieht durch das Anlegen von Fehlerkorrektur-Daten.
Diese werden entweder dem Datenträger hinzugefügt oder
in zusätzlichen Fehlerkorrektur-Dateien aufbewahrt. 
dvdisaster arbeitet auf der Abbild-Ebene und ist dadurch
vom Dateisystem des Datenträgers unabhängig.
Die maximale Kapazität der Fehlerkorrektur ist einstellbar.<p>

<b>Häufige Mißverständnisse über dvdisaster:</b>

<ul>
<li>dvdisaster kann defekte Datenträger nicht wieder lesbar machen.
Ohne Fehlerkorrektur-Daten kann ein beschädigter Datenträger <i>nicht</i> 
wiederhergestellt werden.<p></li> 
<li><img src="images/exclude_from_search_terms.png" alt="" align="middle"><br>
Solche Funktionen sind mit den Zielen und dem inneren Aufbau von dvdisaster nicht vereinbar.</li>
</ul>

<p>
<a href="index10.php">Beispiele für die Fehlerkorrektur...</a>

<?php
# Adds the footer line and closes the HTML properly.

end_page();
?>
