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
require("../include/screenshot.php");

begin_page();
?>

<!--- Insert actual page content below --->

<h3>Ausgaben für Abbilder mit Fehlerkorrektur-Daten</h3>

<?php begin_howto_shot("Abbild mit Fehlerkorrektur-Daten.","compat-okay-rs02.png", ""); ?>
Beim Vergleich eines Abbilds mit den darin enthaltenen Fehlerkorrektur-Daten
erhalten sie die Informationen aufgeteilt über:
<ul>
<li>das gesamte Abbild und</li>
<li>die Fehlerkorrektur-Informationen:</li>
<?php end_howto_shot(); ?>

<table>
<tr><td colspan="2">Ausgabefeld <b>"Abbild-Datei":</b><br><hr></td><td></td></tr>
<tr>
<td valign="top">Datentr.-Sektoren:</td>
<td>Die Anzahl der Sektoren in dem erweiterten ISO-Abbild (also einschließlich
der Fehlerkorrektur-Daten; ein Sektor = 2KB).</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td valign="top">Daten-Prüfsumme:</td>
<td>Die MD5-Prüfsumme des ISO-Abbilds vor der Erweiterung mit
Fehlerkorrektur-Daten.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td>
Ecc-Vorspänne:<br>
Daten-Abschnitt:<br>
Crc-Abschnitt:<br>
Ecc-Abschnitt:
</td>
<td valign="top">Ein mit Fehlerkorrektur-Daten erweitertes Abbild besteht
aus drei Abschnitten, in denen zusätzlich noch Ecc-Vorspänne eingebettet
sind. Diese Werte geben an, wieviele Sektoren in den jeweiligen Bereichen
unlesbar sind.
</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr><td colspan="2">
Wenn alle Werte in diesem Ausgabefeld in Ordnung sind, erhalten Sie
darunter die Ausgabe "<font color="#008000">Gutes Abbild.</font>".
Anderenfalls wird dort der wichtigste aufgetretene Fehler näher erklärt.
</td>
</tr>

<tr><td>&nbsp;</td><td></td></tr>
<tr><td colspan="2">Ausgabefeld <b>"Fehlerkorrektur-Daten":</b><br><hr></td><td></td></tr>
<tr>
<td valign="top">Erzeugt von:</td>
<td>Die dvdisaster-Version, mit der die Fehlerkorrektur-Daten erzeugt wurden.
Alpha/Entwicklerversion werden rot hervorgehoben.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td valign="top">Methode:</td>
<td>Die Methode und Redundanz, mit der die Fehlerkorrektur-Daten erzeugt
wurden.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td valign="top">Benötigt:</td>
<td>Zur Verwendung der Fehlerkorrektur-Daten wird mindestens die
angegebene dvdisaster-Version benötigt.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td valign="top">Datentr.-Sektoren:</td>
<td>Der erste Wert ist die Anzahl der Sektoren in dem erweiterten Abbild;
der zweite Wert beschreibt die Anzahl der Sektoren in dem Abbild vor
der Erweiterung mit Fehlerkorrektur-Daten. Da die Fehlerkorrektur-Daten
hinter den Nutzerdaten stehen, kann man die Daten-Prüfsumme unter
GNU/Linux mit der Kommandozeile wie folgt berechnen:<br>
<tt>head -c $((2048*121353)) abbild.iso | md5sum</tt><br>
Der erste Aufrufwert für <i>head</i> ist die  Sektorgröße (2048) 
multipliziert mit der ursprünglichen Abbildgröße (121353). Dies kann man
auch nutzen, um die Fehlerkorrektur-Daten wieder abzuschneiden:<br>
<tt>head -c $((2048*121353)) abbild.iso >daten_abbild.iso</tt>
</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td valign="top">Daten-Prüfsumme:</td>
<td>Die MD5-Prüfsumme des ursprünglichen Abbilds (siehe vorherige Erklärung).</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td valign="top">
CRC-Prüfsumme:<br>
ECC-Prüfsumme:</td>
<td>MD5-Prüfsummen der CRC- und ECC-Abschnitte des erweiterten Abbilds.
Diese beiden Prüfsummen lassen sich nicht außerhalb von dvdisaster
nachrechnen.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

</tr>
<tr><td colspan="2">
Wenn alle Werte in diesem Ausgabefeld in Ordnung sind, erhalten Sie
darunter die Ausgabe "<font color="#008000">Gute Fehlerkorrektur-Daten.</font>".
Anderenfalls wird dort der wichtigste aufgetretene Fehler näher erklärt.
</td>
</tr>

</table>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
