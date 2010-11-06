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
require("../include/screenshot.php");
begin_page();
?>

<!--- Insert actual page content below --->

<h3>Kompatibilität zwischen Fehlerkorrektur-Dateien und ISO-Abbildern prüfen</h3>


<b>Motivation:</b> Sie möchten einen Datenträger brennen und gleich
dazu eine Fehlerkorrektur-Datei erstellen.  Aus Gründen der Zeitersparnis gehen
Sie wie folgt vor:

<ol>
<li>Sie erstellen ein ISO-Abbild mit Ihrer Brennsoftware.</li>
<li>Sie brennen daraus den Datenträger.</li>
<li>Sie erzeugen aus diesem Abbild die Fehlerkorrektur-Datei.</li>
</ol>

<b>Mögliche Inkompatibilität:</b> Die Brennsoftware erzeugt einen Datenträger,
der dem ISO-Abbild nicht genau entspricht. Dies kann dazu führen, 
daß die Fehlerkorrektur später nicht in der Lage ist, den Inhalt des defekten
Datenträger wiederherzustellen.<p>

<b>So überprüfen Sie die Kompatibilität:</b><p>

Bitte beachten Sie daß einige Schritte hier nur skizziert werden;
folgen Sie den Querverweisen zu ausführlichen Beschreibungen und Beispielen
in den jeweiligen Unterkapiteln dieser Dokumentation. <p>

<table>
<tr>
<td width="200px" align="center"><img src="../images/good-image.png">
<p><img src="../images/down-fork-arrow.png"></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Erzeugen Sie ein ISO-Abbild der Daten</b>, die Sie auf einen
Datenträger brennen möchten. Falls Sie nicht wissen, wie Sie mit Ihrer Brennsoftware
ein ISO-Abbild erzeugen können, schauen Sie in das
<a href="howtos33.php?way=1">Beispiel zum Erzeugen von ISO-Abbildern</a>.
</td>
</tr>
</table>

<table>
<tr>
<td width="100px" align="center">
<img src="../images/good-cd.png" border="0" align="center"><p>
<img src="../images/down-arrow.png" border="0">
</td>
<td width="100px" align="center" valign="top">
<img src="../images/ecc.png" border="0" align="center">
</td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Datenträger brennen und Fehlerkorrektur-Datei erzeugen.</b>
Verwenden Sie das gerade erzeugte ISO-Abbild, um 
den <a href="howtos33.php?way=3#c">Datenträger zu brennen</a>. Nehmen Sie
anschließend <a href="howtos22.php#ecc">diese Grundeinstellungen</a> vor und
<a href="howtos23.php?way=2">erzeugen Sie eine Fehlerkorrektur-Datei</a>
aus dem Abbild.
</td>
</tr>
</table>

<table>
<tr>
<td width="100px" align="center">
<img src="../images/good-image2.png" border="0"><p>
<img src="../images/down-arrow.png" border="0">
</td>
<td width="100px" align="center"> </td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Erstellen Sie ein <i>zweites</i> Abbild aus dem <i>gebrannten</i> Datenträger.</b> Verwenden Sie die gleichen <a href="howtos22.php#read">Grundeinstellungen</a>
und das gleiche Vorgehen wie beim <a href="howtos23.php?way=1">Einlesen eines Datenträgers</a> zum Erstellen einer Fehlerkorrektur-Datei. Allerdings brauchen Sie die
Fehlerkorrektur-Datei nicht noch einmal zu erzeugen.
</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center">
<a href="howtosa2.php">
<img src="../images/select-image.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Geben Sie den Dateinamen des <i>zweiten</i> ISO-Abbilds</b> an, 
das Sie eben von dem Datenträger eingelesen haben. Beachten Sie daß der folgende
Test nutzlos ist wenn Sie mit dem ersten Abbild arbeiten, das Sie mit der 
Brennsoftware erstellt haben.
</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center">
<a href="howtosa3.php">
<img src="../images/select-ecc.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top">
<b>Geben Sie außerdem den Namen der Fehlerkorrektur-Datei</b> an,
sofern dieser nicht ohnehin noch voreingestellt ist.
</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center">
<a href="howtosa4.php">
<img src="images/compare-icon.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Beginnen Sie die Auswertung</b> durch
Klicken auf den "Vergleichen"-Knopf.</td>
</tr>
</table>

<?php begin_howto_shot("Informationen anzeigen.","compat-okay-rs01.png", ""); ?>
<b>Betrachten Sie das Ergebnis des Vergleichs.</b>
Wenn Sie die grünen Meldungen "Gutes Abbild" und "Gute Fehlerkorrektur-Datei"
erhalten, sind die Brennsoftware und dvdisaster bezüglich der ISO-Abbilder
kompatibel. Sie können in Zukunft Ihre Fehlerkorrektur-Dateien direkt aus
den ISO-Abbildern der Brennsoftware erzeugen.
<?php end_howto_shot(); ?>

<hr>

<a name="err"> </a>
<b>Mögliche Fehlerursachen und -behebungen:</b><p>

<?php begin_howto_shot("Falsche Abbildgröße.","compat-150-rs01.png", "down-arrow.png"); ?>
<b>Typisches Problem: Falsche Abbildgröße.</b>
Der Vergleich wird möglicherweise herausfinden daß das Abbild länger als erwartet
ist. Typisch sind Werte von 150 oder 300 Sektoren bei CD oder 1-15 Sektoren
bei DVD. Es ist möglich, daß die Brennsoftware aus technischen Gründen einfach
einige leere Sektoren an das Abbild angehängt hat. Führen Sie die folgenden
Schritte aus um herauszufinden ob dies der Fall ist:
<?php end_howto_shot(); ?>

<table>
<tr>
<td width="200px" align="center">
<img src="images/fix-icon.png" border="0">
<p><img src="../images/down-arrow.png" border="0"></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Beginnen Sie einen Reparaturvorgang.</b>
</td>
</tr>
</table>

<?php begin_howto_shot("Abbild verkürzen.","compat-dialog-rs01.png", "down-arrow.png"); ?>
<b>Bestätigen Sie den Dialog.</b>
Es wird ein Dialog erscheinen, in denen Sie gefragt werden ob das Abbild
um die überzähligen Sektoren verkürzt werden soll. Bestätigen Sie mit "OK".
<?php end_howto_shot(); ?>


<table>
<tr>
<td width="200px" align="center">
<img src="images/stop-icon.png" border="0">
<p><img src="../images/down-arrow.png" border="0"></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Brechen Sie den Reparaturvorgang ab,</b>
da es außer der Verkürzung des Abbilds nichts weiter zu tun gibt.
</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center">
<a href="howtosa4.php">
<img src="images/compare-icon.png" border="0">
<p><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Beginnen Sie die Auswertung</b> erneut durch
Klicken auf den "Vergleichen"-Knopf.</td>
</tr>
</table>

<?php begin_howto_shot("Informationen anzeigen.","compat-okay-rs01.png", ""); ?>
<b>Betrachten Sie das Ergebnis des erneuten Vergleichs.</b>
Wenn Sie jetzt die grünen Meldungen "Gutes Abbild" und "Gute Fehlerkorrektur-Datei"
erhalten, haben Sie tatsächlich nur ein kosmetisches Problem: Die Brennsoftware
hat einige leere Sektoren am Ende des Abbilds beim Brennen hinzugefügt.
<?php end_howto_shot(); ?>

<font color="#800000">Wenn das Problem trotz der obigen Schritte bestehen bleibt,
können Sie <i>nicht</i> davon ausgehen, daß dvdisaster und die Brennsoftware
kompatibel sind. Die erzeugten Fehlerkorrektur-Dateien werden voraussichtlich
unbrauchbar sein.</font> <p> 
Erzeugen Sie im Fall der Inkompatibilität Ihre Fehlerkorrektur-Dateien mit der
nachfolgend beschriebenen Methode:

<hr>

<pre> </pre>

<b>Alternative: So vermeiden Sie mögliche Inkompatibilitäten:</b>

<ol>
<li>Brennen Sie erst den Datenträger.</li>
<li>Erstellen das ISO-Abbild mit dvdisaster von dem gebrannten Datenträger.</li>
<li> Erzeugen Sie aus diesem Abbild die Fehlerkorrektur-Datei.</li>
</ol>
Dieses Vorgehen ist durch den zusätzlichen Lesevorgang langsamer, hat allerdings
auch den Vorteil daß der gebrannte Datenträger einmal komplett auf Lesbarkeit
überprüft wird.


<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
