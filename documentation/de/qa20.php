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

<h3 class="top">Fehlermeldungen und Probleme</h3>

<a href="#tao">3.1 Was bedeutet "Warnung: 2 Sektoren fehlen am Ende des Datenträgers..."?</a><p>
<a href="#block">3.2 Das Programm hängt nach dem Aufruf.</a><p>
<a href="#crc">3.3 Was bedeutet die Meldung "CRC error, sector: n"?</a><p>
<a href="#rw">3.4 Lesefehler oder falsche Abbild-Größe bei -RW/+RW/-RAM-Datenträgern</a><p>
<a href="#dvdrom">3.5 Selbstgebrannter Datenträger wird als "DVD-ROM" erkannt und abgelehnt.</a><p>
<a href="#freebsd">3.6 Unter FreeBSD erscheinen keine Laufwerke.</a><p>
<a href="#v40error">3.7 "Fehlerkorrekturdatei wurde mit Version 0.40.7 erzeugt"</a><p>

<hr><p>

<b><a name="tao">3.1 Was bedeutet "Warnung: 2 Sektoren fehlen am Ende des Datenträgers..."?</a></b><p>
Diese Warnung tritt bei CD-Datenträgern auf, die im "TAO"-Modus ("track at once")
gebrannt wurden. Manche Laufwerke liefern für diese Medien eine um 2 zu große Länge
für das Abbild zurück. Dadurch entstehen 2 Pseudo-Lesefehler am Ende des Datenträgers, 
die jedoch <i>keinen</i> Datenverlust bedeuten. <p>

Da man dem Datenträger nicht ansehen kann, in welcher Betriebsart er gebrannt wurde,
geht dvdisaster davon aus, daß eine "TAO"-CD vorliegt, wenn nur die letzten beiden
Sektoren unlesbar sind, und das Abbild wird um diese beiden Sektoren verkürzt.
Ob dies zutreffend ist, müssen Sie von Fall zu Fall selbst entscheiden
und gegebenenfalls mit der --dao -Option 
oder dem Dialog zum Lesen und Prüfen einen 
Hinweis geben, um diese Sektoren als echte Lesefehler zu betrachten.<p>

Wenn Sie Datenträger mit nur einer Sitzung erzeugen, sollten Sie daher immer
im Modus "DAO / Disc at once" (manchmal auch "SAO / Session at once" genannt) brennen, 
um diese Probleme zu vermeiden. 
<div class="talignr"><a href="#top">&uarr;</a></div>


<b><a name="block">3.2 Das Programm hängt nach dem Aufruf</a></b><p>
Bei alten Linux-Kernel-Versionen (2.4.x) bleibt das Programm 
manchmal nach dem Starten hängen, bevor es mit der ersten
Aktion beginnt. Es läßt sich dann weder mit Strg-C noch mit "kill -9"
unterbrechen.<p>

Werfen Sie den Datenträger aus, damit sich das Programm beendet.
Legen Sie das Speichermedium dann wieder ein und warten Sie, 
bis das Laufwerk den Datenträger erkennt und zur Ruhe kommt.
Ein erneuter Aufruf von dvdisaster sollte jetzt funktionieren. 
<div class="talignr"><a href="#top">&uarr;</a></div>


<b><a name="crc">3.3 Was bedeutet die Meldung "CRC error, sector: n"?</a></b><p>
Der betreffende Sektor konnte gelesen werden, aber die Prüfsumme seines Inhalts
stimmt nicht mit ihrem Gegenstück in der Fehlerkorrekturdatei überein. 
Für die Ursachen gibt es <a href="howtos13.php?crc">mehrere mögliche Erklärungen</a>. 
<div class="talignr"><a href="#top">&uarr;</a></div>

<b><a name="rw">3.4 Lesefehler oder falsche Abbild-Größe bei -RW/+RW/-RAM-Datenträgern</a></b><p>

Einige Laufwerke liefern bei -RW/+RW/-RAM-Datenträgern fehlerhafte Informationen über die
Abbild-Größe. Besonders häufig sind die folgenden beiden Fälle:<p>

<table>
<tr><td class="valignt">Fehler:</td>
<td>Das Laufwerk liefert den Umfang des größten jemals auf den Datenträger geschriebenen Abbildes 
anstelle der tatsächlichen Abbild-Größe.
</td></tr>
<tr><td class="valignt">Auswirkung:</td>
<td>Ein Datenträger wird gelöscht und dann mit einer 100MB großen Datei beschrieben.
Beim Zurücklesen ist das Abbild aber einige GB groß und es enthält
noch die Reste älterer Abbilder.
</td></tr>
<tr><td><pre> </pre></td><td></td></tr>
<tr><td class="valignt">Fehler:</td>
<td>Das Laufwerk liefert die maximale Datenträger-Kapazität (typischerweise 2295104 Sektoren)
anstelle der tatsächlich genutzten Sektoren.
</td></tr>
<tr><td class="valignt">Auswirkung:</td>
<td>Beim Einlesen des Abbilds treten ab einer bestimmten Stelle nur noch Lesefehler auf;
die Dateien auf dem Datenträger sind aber alle vollständig.
</td></tr>
</table><p>

Mögliche Abhilfe: <p>

<table width="100%"><tr><td class="vsep"></td><td>
Verwenden Sie die Option zum Bestimmen der Abbildgröße 
aus dem ISO/UDF- bzw. ECC/RS02 Dateisystem.
</td></tr></table><p>

Falls bei einem beschädigten Datenträgers die benötigten ISO/UDF-Sektoren 
auf dem Abbild unlesbar sind und eine Fehlerkorrektur-Datei verwendet wird, 
haben Sie zwei Möglichkeiten:

<ul>
<li>Führen Sie die <a href="howtos50.php">"Vergleichen"</a>-Funktion nur mit der 
Fehlerkorrektur-Datei aus. Entnehmen Sie die korrekte Abbild-Größe der Ausgabe und 
schränken Sie den Lesebereich entsprechend ein.
</li>
<li>Lesen Sie einfach das Abbild mit der zu großen Länge ein. Wenn Sie nach dem Aufruf
der <a href="howtos40.php#repair">"Reparieren"</a>-Funktion gefragt werden, ob das Abbild
abgeschnitten werden soll, antworten Sie mit "OK".
</li>
</ul>

<div class="talignr"><a href="#top">&uarr;</a></div>


<b><a name="dvdrom">3.5 Selbstgebrannter Datenträger wird als "DVD-ROM" erkannt und abgelehnt.</a></b><p>

Wahrscheinlich wurde der "book type" des Rohlings beim Brennen auf "DVD-ROM" gesetzt.
dvdisaster kann solche Datenträger typischerweise nur auf Laufwerken verarbeiten,
die das entsprechende Format auch schreiben können.<p>

Eine zweischichtige DVD+R mit falschem "book type" wird zum Beispiel nur auf einem 
Brenner angenommen, der auch solche Rohlinge schreiben kann.<p>

Versuchen Sie in diesen Fällen, das Abbild mit einem anderen Laufwerk einzulesen.

<div class="talignr"><a href="#top">&uarr;</a></div>


<b><a name="freebsd">3.6 Unter FreeBSD erscheinen keine Laufwerke.</a></b><p>

<ul>
<li>Unter FreeBSD wird für ATAPI-Laufwerke (das sind fast alle heute gebräuchlichen Typen)
möglicherweise ein <a href="download10.php#freebsd">Neuübersetzen des Kernels</a>
benötigt, um die Laufwerke mit dvdisaster verwenden zu können. 
<li>Sie müssen Lese- und Schreibrechte auf dem betreffenden Gerät 
(z.B. /dev/pass0) haben.
</ul>

<div class="talignr"><a href="#top">&uarr;</a></div>


<b><a name="v40error">3.7 "Fehlerkorrekturdatei wurde mit Version 0.40.7 erzeugt"</a></b><p>

Die <a href="http://sourceforge.net/cvs/?group_id=157550">CVS-Versionen</a> von 
dvdisaster markieren ihre Fehlerkorrektur-Dateien mit einem
speziellen Bit. Dies bewirkt in den dvdisaster-Versionen bis einschließlich
0.65 fälschlicherweise die obige Fehlermeldung. Bitte verwenden Sie die CVS-Versionen
nur zusammen mit dvdisaster 0.66 oder neueren Versionen.

<div class="talignr"><a href="#top">&uarr;</a></div>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
