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

<h3>Zusätzliche Papiere</h3>

Die Online-Dokumentation, die Sie gerade lesen, ist auch
in den dvdisaster-Programmpaketen selbst enthalten. Sie brauchen
sich diese Seiten also nicht zusätzlich herunterzuladen.<p>

Folgende zusätzliche Papiere sind verfügbar:<p>

<b>Spezifikation für RS03</b><p>

Mit RS03 wird in zukünftigen dvdisaster-Versionen ein neues
Kodierungsverfahren eingeführt, das seine Berechnungen auf mehrere
Prozessorkerne verteilen kann. Dies ist mit den momentanen 
Verfahren RS01 und RS02 aufgrund deren inneren Aufbaus nicht möglich. <p>

Eine <a href="http://dvdisaster.net/papers/rs03.pdf">Vorschau der RS03-Spezifikation (rs03.pdf)</a>
ist ab sofort verfügbar, um die Eigenschaften des neuen Kodierers
diskutieren zu können. Die Spezifikation ist noch nicht endgültig.<p>

Das RS03-Dokument setzt Kenntnisse im Bereich Kodierungstheorie voraus
und ist nicht als Benutzerdokumentation gedacht.


<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
