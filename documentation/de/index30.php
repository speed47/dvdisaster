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

<h3>Vorteile von dvdisaster:</h3>

<ul>
<li><b>Schutz</b> vor unvorhergesehener Beschädigung (in bestimmten Grenzen).<p></li>
<li><a href="howtos10.php">Überprüfen auf Lesefehler</a> geht <b>schneller</b> als 
eine Qualitäts-Analyse; bei vielen Laufwerken mit voller Lesegeschwindigkeit.<p></li>
<li><b>Kostensparend:</b> Datenträger brauchen erst dann umkopiert zu werden,
wenn sie wirklich defekt sind.</li>
</ul>

<h3>Nachteile von dvdisaster:</h3>

Sie benötigen eine Datensicherungs-Strategie und mindestens 15% zusätzlichen
Speicherplatz.

<ul>
<li>Fehlerkorrektur-Daten <b>müssen erzeugt werden, bevor</b> der 
Datenträger kaputt geht (am besten gleich beim Brennen des Datenträgers).<p></li>
<li>Die Fehlerkorrektur-Daten benötigen <b>zusätzlichen Speicherplatz</b>;
entweder direkt auf dem zu schützenden Datenträger oder auf zusätzlichen
Datenträgern.
In der Grundeinstellung beträgt der
Speicherplatzbedarf 15% der Originaldaten (ca. 700MB für eine volle einschichtige DVD).<p></li>
<li>kein garantierter Schutz vor Datenverlust.</li>
</ul>

<p>
Schauen Sie auch in die  <a href="http://dvdisaster.net/legacy/de/background.html">Hintergrundinformationen</a> in der alten Dokumentation, um mehr über die
Arbeitsweise von dvdisaster zu erfahren.

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
