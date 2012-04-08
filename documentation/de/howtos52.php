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
require("../include/screenshot.php");

begin_page();
?>

<!-- Insert actual page content below -->

<h3 class="top">Ausgaben für Fehlerkorrektur-Dateien</h3>

<?php begin_howto_shot("Abbild und Fehlerkorrektur-Datei.","compat-okay-rs01.png", ""); ?>

Beim Vergleich eines Abbilds mit der zugehörigen Fehlerkorrektur-Datei
erhalten Sie die Informationen über den Zustand des Abbilds und der
Fehlerkorrektur-Datei in zwei getrennten Ausgabefeldern:
<?php end_howto_shot(); ?>

<table>
<tr><td colspan="2">Ausgabefeld <b>"Abbild-Datei":</b><br><hr></td><td></td></tr>
<tr>
<td class="valignt">Datentr.-Sektoren:</td>
<td>Die Anzahl der Sektoren in dem ISO-Abbild (ein Sektor = 2KB).</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>
<tr>
<td class="valignt">Prüfsummen-Fehler:</td>
<td>Die Fehlerkorrektur-Datei enthält eine CRC32-Prüfsumme für jeden
Sektor im Abbild. Wenn dieser Wert größer als Null ist, sind die Daten
in einigen Sektoren lesbar, aber nicht so wie es die Prüfsumme erwarten 
läßt. Die Fehlerkorrektur wird versuchen den Inhalt dieser Sektoren neu
zu berechnen.
</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td class="valignt">Fehlende Sektoren:</td>
<td>Dies ist die Anzahl der Sektoren, die nicht von dem Datenträger 
gelesen worden konnten. Die Fehlerkorrektur wird versuchen, den Inhalt
dieser Sektoren wiederherzustellen.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td class="valignt">Abbild-Prüfsumme:</td>
<td>Das komplette ISO-Abbild ist noch einmal mit einer MD5-Prüfsumme
versehen. Sie können diesen Wert auch selbst (unter GNU/Linux) in
der Kommandozeile berechnen:<br>
<tt>md5sum abbild2.iso</tt></td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr><td colspan="2">
Wenn alle Werte in diesem Ausgabefeld in Ordnung sind, erhalten Sie
darunter die Ausgabe "<span class="green">Gutes Abbild.</span>".
Anderenfalls wird dort der wichtigste aufgetretene Fehler näher erklärt.
</td>
</tr>

<tr><td>&nbsp;</td><td></td></tr>
<tr>
<td colspan="2">Ausgabefeld <b>"Fehlerkorrektur-Datei"</b>:<br><hr></td><td></td>
</tr>
<tr>
<td class="valignt">Erzeugt von:</td>
<td>Die dvdisaster-Version, mit der die Fehlerkorrektur-Datei erzeugt wurde.
Alpha/Entwicklerversion werden rot hervorgehoben.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td class="valignt">Methode:</td>
<td>Die Methode und Redundanz, mit der die Fehlerkorrektur-Datei erzeugt
wurde.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td class="valignt">Benötigt:</td>
<td>Zur Verwendung der Fehlerkorrektur-Daten wird mindestens die
angegebene dvdisaster-Version benötigt.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td class="valignt">Datentr.-Sektoren:</td>
<td>Die erwartete Anzahl der Sektoren in der zugehörigen Abbild-Datei.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td class="valignt">Abbild-Prüfsumme:</td>
<td>Die erwartete MD5-Prüfsumme der zugehörigen Abbild-Datei.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td class="valignt">Fingerabdruck:</td>
<td>dvdisaster verwendet die Prüfsumme eines bestimmten Sektors
um festzustellen ob die Fehlerkorrektur-Datei und das Abbild zusammenpassen.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td class="valignt">Ecc-Bereiche:</td>
<td>Die Fehlerkorrektur unterteilt das Abbild in kleine Bereiche, die
unabhängig voneinander korrigiert werden können. Wichtig ist eigentlich nur,
daß diese Zahl stimmt ;-)</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr>
<td class="valignt">Ecc-Prüfsumme:</td>
<td>Eine MD5-Prüfsumme über die Fehlerkorrektur-Datei, abzüglich der
ersten 4KB. Sie können diesen Wert unter GNU/Linux mit der Kommandozeile
wie folgt berechnen:<br>
<tt>tail -c +4097 abbild.ecc | md5sum</tt>
</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>

<tr><td colspan="2">
Wenn alle Werte in diesem Ausgabefeld in Ordnung sind, erhalten Sie
darunter die Ausgabe "<span class="green">Gute Fehlerkorrektur-Datei.</span>".
Anderenfalls wird dort der wichtigste aufgetretene Fehler näher erklärt.
</td>
</tr>
</table>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
