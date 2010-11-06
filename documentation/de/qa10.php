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

<h3><a name="top">Technische Fragen</a></h3>

<a href="#nls">2.1 In welchen Übersetzungen ist das Programm verfügbar?</a><p>
<a href="#media">2.2 Welche Datenträger-Typen werden unterstützt?</a><p>
<a href="#filesystem">2.3 Welche Dateisysteme werden unterstützt?</a><p>

<hr><p>

<b><a name="nls">2.1 In welchen Übersetzungen ist das Programm verfügbar?</a></b><p>

dvdisaster beinhaltet Bildschirmtexte in den folgenden Sprachen:<p>

<table>
<tr><td>&nbsp;&nbsp;&nbsp;</td><td>Deutsch</td><td>--</td><td>vollständig</td></tr>
<tr><td></td><td>Englisch</td><td>--</td><td>vollständig</td></tr>
<tr><td></td><td>Italienisch</td><td>--</td><td>bis Version 0.65</td></tr>
<tr><td></td><td>Portugiesisch</td><td>--</td><td>vollständig</td></tr>
<tr><td></td><td>Russisch</td><td>--</td><td>vollständig</td></tr>
<tr><td></td><td>Schwedisch</td><td>--</td><td>vollständig</td></tr>
<tr><td></td><td>Tschechisch</td><td>--</td><td>bis Version 0.66</td></tr>
</table><p>

Übersetzer/-innen für andere Sprachen sind willkommen!<p>

dvdisaster übernimmt die Spracheinstellungen des Betriebssystems
automatisch. Falls die lokale Sprache noch nicht unterstützt wird,
werden englische Texte verwendet. Eine andere Sprache kann mit Hilfe
von Umgebungsvariablen eingestellt werden.<p>

Zum Beispiel für die bash-Kommandozeile und deutsche Sprache:

<pre>export LANG=de_DE</pre>

Wenn Umlaute nicht richtig oder als "a,"o,"u usw. dargestellt werden,
fehlt eventuell noch: <p>

<tt>export OUTPUT_CHARSET=iso-8859-1</tt> (X11, XTerm)

<div align=right><a href="#top">&uarr;</a></div>



<b><a name="media">2.2 Welche Datenträger-Typen werden unterstützt?</a></b><p>

dvdisaster unterstützt einmal oder mehrmals beschreibbare 
CD-, DVD- und BD-Datenträger.
Datenträger mit Mehrfachsitzungen (engl.: multisession)
oder einem Kopierschutz können <i>nicht</i> verwendet werden.<p>

Unterstützte Datenträger nach Typ:<p>

<b>CD-R, CD-RW</b><p>

<ul>
 <li>nur Daten-CDs werden unterstützt.</li>
</ul>

<b>DVD-R, DVD+R</b><p>

<ul>
<li>Keine weiteren Einschränkungen bekannt.</li>
</ul>

<b>DVD-R DL, DVD+R DL (zweischichtig)</b>
<ul>
<li>
Das Laufwerk muß die <a href="qa20.php#dvdrom">Erkennung
des Datenträger-Typs</a> ermöglichen. Dies erfordert typischerweise 
Laufwerke, die auch zweischichtige Medien beschreiben können.
</li>
</ul>

<b>DVD-RW, DVD+RW</b><p>

<ul>
<li>Einige Laufwerke liefern eine <a href="qa20.php#rw">falsche Abbild-Größe</a>.<br>
Abhilfe: Abbildgröße aus dem ISO/UDF- oder ECC/RS02-Dateisystem ermitteln.
</li>
</ul>

<b>DVD-RAM</b><p>
<ul>
<li>Müssen wie DVD-R/-RW mit einem ISO/UDF-Abbild beschrieben sein.</li>
<li>Keine Unterstützung bei Einsatz als Wechselspeichermedium / packet writing.</li>
<li>Ähnliche Probleme mit der Erkennung der <a href="qa20.php#rw">Abbild-Größe</a>
wie oben beschrieben möglich.</li>
</ul>

<b>BD-R, BD-RW</b><p>

<ul>
<li>Momentan sind keine Einschränkungen bekannt, aber mit
zweischichtigen Typen (die 50GB-Versionen) liegen noch kaum
Erfahrungen vor.</li>
</ul>

<b>Nicht verwendbare Typen</b> (kein Einlesen des Abbildes möglich):<p> 
BD-ROM (gepreßte BDs), DVD-ROM (gepreßte DVDs) sowie Audio-CD und Video-CD.

<div align=right><a href="#top">&uarr;</a></div><p>


<b><a name="filesystem">2.3 Welche Dateisysteme werden unterstützt?</a></b><p>

dvdisaster arbeitet ausschließlich auf der Ebene von Datenträger-Abbildern,
auf die es sektorweise zugreift. 
Daher ist es unerheblich, mit welchem Dateisystem der Datenträger formatiert wurde. <p>
Weil dvdisaster die Struktur der Dateisysteme nicht kennt bzw. nutzt,
kann es keine logischen Fehler darin beheben und insbesondere keine
verlorengegangenen oder gelöschten Dateien wiederherstellen.
<div align=right><a href="#top">&uarr;</a></div><p>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
