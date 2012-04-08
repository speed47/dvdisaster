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

<h3 class="top">Kompatibilität beim Ablegen von Fehlerkorrektur-Daten auf dem Datenträger prüfen</h3>

<b>Motivation:</b> dvdisaster kann Fehlerkorrektur-Daten
<a href="howtos30.php">zusammen mit den Nutzdaten auf dem Datenträger</a>
unterbringen. Die Fehlerkorrektur-Daten werden dabei so dem ISO-Abbild
hinzugefügt, daß sie für andere Anwendungen unsichtbar sind und diese
somit nicht stören.<p>

<b>Mögliche Inkompatibilität:</b> Die Brennsoftware sieht die
Fehlerkorrektur-Daten auch nicht. Es ist nicht wahrscheinlich, 
aber möglich daß sie die Fehlerkorrektur-Daten beim Brennen abschneidet
oder beschädigt. In diesem Fall würde die Fehlerkorrektur nicht 
funktionieren.<p>


<b>So überprüfen Sie die Kompatibilität:</b><p>

Bitte beachten Sie daß die nötigen Schritte hier nur skizziert werden;
folgen Sie den Querverweisen zu ausführlichen Beschreibungen und Beispielen
in den jeweiligen Unterkapiteln dieser Dokumentation. <p>

<table>
<tr>
<td class="w200x" align="center"><img src="../images/good-cd-ecc.png" alt="Symbol: Datenträger mit Fehlerkorrektur-Daten">
<p><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Erzeugen Sie zunächst einen Datenträger, der mit
Fehlerkorrektur-Daten erweitert</b> wurde. 
Vergessen Sie nicht, die erforderlichen <a href="howtos32.php">Einstellungen</a>
vorzunehmen und folgen Sie der
<a href="howtos33.php">Schritt-für-Schritt</a>-Anleitung.<br> 
Verwenden Sie für diesen Test keine wiederbeschreibbaren
DVD- oder BD-Datenträger, da dies unter bestimmten Umständen den Testausgang
beeinflussen kann
(siehe <a href="qa20.php#rw">Punkt 3.4 in den Fragen und Antworten</a>).
</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center">
<img src="../images/good-image2.png" alt="Symbol: Vollständiges Abbild von dem eben geschriebenen Datenträger" class="noborder"><p>
<img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten" class="noborder">
</td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Erstellen Sie ein <i>zweites</i> Abbild aus dem <i>gebrannten</i> Datenträger.</b> Verwenden Sie die gleichen <a href="howtos22.php">Grundeinstellungen</a>
und das gleiche Vorgehen wie beim <a href="howtos23.php?way=1">Einlesen eines Datenträgers</a> zum Erstellen einer Fehlerkorrektur-Datei. 
Allerdings brauchen wir nur die Abbild-Datei; es muß keine 
Fehlerkorrektur-Datei erzeugt werden.
</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center">
<a href="howtosa2.php">
<img src="../images/select-image.png" alt="Bedienelement: Abbild-Datei auswählen (Eingabefeld und Knopf für Dialog)" class="noborder">
<br><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten" class="noborder"></a></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Geben Sie den Dateinamen des <i>zweiten</i> ISO-Abbilds</b> an, 
das Sie eben von dem Datenträger eingelesen haben. Beachten Sie daß der folgende
Test nutzlos ist wenn Sie mit dem ersten Abbild arbeiten, das Sie mit der 
Brennsoftware erstellt und mit dvdisaster erweitert haben.
</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center">
<a href="howtosa4.php">
<img src="images/compare-icon.png" alt="Bedienelement: Vergleichen (Auswahlknopf)" class="noborder">
<br><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten" class="noborder"></a></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Beginnen Sie die Auswertung der Abbild-Datei</b> durch
Klicken auf den "Vergleichen"-Knopf.</td>
</tr>
</table>

<?php begin_howto_shot("Informationen anzeigen.","compat-okay-rs02.png", ""); ?>
<b>Betrachten Sie das Ergebnis des Vergleichs.</b>
Wenn Sie die grünen Meldungen "Gutes Abbild" und "Gute Fehlerkorrektur-Daten"
erhalten, sind die Brennsoftware und dvdisaster bezüglich der 
erweiterten ISO-Abbilder kompatibel. Sie können in Zukunft Ihre mit 
Fehlerkorrektur-Daten erweiterten Abbilder mit dieser Brennsoftware
auf Datenträger schreiben.
<?php end_howto_shot(); ?>

<hr>

<a name="err"> </a>
<b>Mögliche Fehlerursachen und -behebungen:</b><p>

<?php begin_howto_shot("Falsche Abbildgröße.","compat-150-rs02.png", "down-arrow.png"); ?>
<b>Typisches Problem: Falsche Abbildgröße.</b>
Der Vergleich wird möglicherweise herausfinden daß das Abbild länger als erwartet
ist. Typisch sind Werte von 150 oder 300 Sektoren bei CD oder 1-15 Sektoren
bei DVD. Es ist möglich, daß die Brennsoftware aus technischen Gründen einfach
einige leere Sektoren an das Abbild angehängt hat. Führen Sie die folgenden
Schritte aus um herauszufinden ob dies der Fall ist:
<?php end_howto_shot(); ?>

<table>
<tr>
<td class="w200x" align="center">
<img src="images/fix-icon.png" alt="Bedienelement: Reparieren (Auswahlknopf)" class="noborder">
<p><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten" class="noborder"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Beginnen Sie einen Reparaturvorgang.</b>
</td>
</tr>
</table>

<?php begin_howto_shot("Abbild verkürzen.","compat-dialog-rs02.png", "down-arrow.png"); ?>
<b>Bestätigen Sie den Dialog.</b>
Es wird ein Dialog erscheinen, in denen Sie gefragt werden ob das Abbild
um die überzähligen Sektoren verkürzt werden soll. Bestätigen Sie mit "OK".
<?php end_howto_shot(); ?>


<table>
<tr>
<td class="w200x" align="center">
<img src="images/stop-icon.png" alt="Bedienelement: Abbrechen (Auswahlknopf)" class="noborder">
<p><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten" class="noborder"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Brechen Sie den Reparaturvorgang ab,</b>
da es außer der Verkürzung des Abbilds nichts weiter zu tun gibt.
</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center">
<a href="howtosa4.php">
<img src="images/compare-icon.png" alt="Bedienelement: Vergleichen (Auswahlknopf)" class="noborder">
<br><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten" class="noborder"></a></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Beginnen Sie die Auswertung</b> erneut durch
Klicken auf den "Vergleichen"-Knopf.</td>
</tr>
</table>

<?php begin_howto_shot("Informationen anzeigen.","compat-okay-rs02.png", ""); ?>
<b>Betrachten Sie das Ergebnis des erneuten Vergleichs.</b>
Wenn Sie jetzt die grünen Meldungen "Gutes Abbild" und "Gute Fehlerkorrektur-Daten"
erhalten, haben Sie tatsächlich nur ein kosmetisches Problem: Die Brennsoftware
hat einige leere Sektoren am Ende des Abbilds beim Brennen hinzugefügt.
<?php end_howto_shot(); ?>

<span class="red">Wenn das Problem trotz der obigen Schritte bestehen 
bleibt, können Sie die betreffende Brennsoftware <i>nicht</i> dazu verwenden,
um mit Fehlerkorrektur-Daten erweiterte Abbilder zu schreiben.
Führen Sie den Test erneut mit der Software eines anderen Herstellers aus.
</span> <p> 

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
