<?php
# dvdisaster: German homepage translation
# Copyright (C) 2004-2009 Carsten Gnörlich
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

<h3>dvdisaster als Ergänzung zu Qualitäts-Analysen</h3>

<a href="qa.php#pipo">Qualitäts-Analysen</a>, z.B. C2- oder PI/PO-Tests,
sind ein wertvolles Werkzeug zur Überprüfung und Optimierung der Brennqualität.<p>

Aber Qualitäts-Analysen können die <b>Lebensdauer</b> von Datenträgern
<b>nicht zuverlässig voraussagen</b>. <br> 
Es ist schwierig, den richtigen Zeitpunkt erwischen, um einen gealterten
Datenträger auf einen neuen umzukopieren:

<ul>
<li>Zu früh: Umkopieren aufgrund schlechter Qualitäts-Werte erzeugt mitunter unnötige
Kosten - manchmal halten solche Datenträger noch viel länger als erwartet.<p></li>
<li>Zu spät: Wenn die Qualitäts-Analyse bereits unlesbare Sektoren anzeigt, sind
schon Daten verloren.<p></li>
<li>Genau bevor der Datenträger kaputt geht: Der ideale Fall, aber praktisch unmöglich
zu treffen.
</ul>

dvdisaster geht daher einen anderen Weg:

<ul>
<li>Man <a href="howtos20.php">erzeugt Fehlerkorrektur-Daten</a> für den Datenträger.<p></li>
<li>Der Datenträger wird <a href="howtos10.php">regelmäßig geprüft</a> und solange verwendet, bis die ersten Fehler auftreten.<p></li>
<li>Unlesbare Sektoren werden durch die <a href="howtos40.php">Fehlerkorrektur-Daten</a>  <a href="howtos30.php">wiederhergestellt</a>. Von dem reparierten Abbild wird ein
neuer Datenträger erstellt.</li>
</ul>

<p>
<a href="index30.php">Vor- und Nachteile von dvdisaster zusammengefaßt...</a>


<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
