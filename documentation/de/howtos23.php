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

$way=$_GET["way"];
switch($way)
{  case 1: $action="aus einem Datenträger"; break;
   case 2: $action="aus einem ISO-Abbild"; break;
   default: $action="Durchführen"; break;
}

howto_headline("Fehlerkorrektur-Datei erstellen", $action, "images/create-icon.png");
?>

<!--- Insert actual page content below --->

Vergewissern Sie sich zunächst, daß dvdisaster wie in den 
<a href="howtos22.php">Grundeinstellungen</a> beschrieben konfiguriert ist.
Ungünstige Einstellungen können dazu führen, 
daß die Fehlerkorrektur-Dateien später keine optimale Wirkung haben.
<p>

Das weitere Vorgehen hängt davon ab, aus welcher Quelle Sie die Fehlerkorrektur-Datei
erzeugen möchten. Klicken Sie auf eine der beiden Möglichkeiten:<p>

<table width="100%" cellspacing="5">
<tr>

<?php
$expand=$_GET["expand"];
if($expand=="") $expand=0;
echo "<td><a href=\"howtos23.php?way=1&expand=$expand\"><img src=\"../images/good-cd.png\" border=\"0\"></a></td>\n";
echo "<td><a href=\"howtos23.php?way=1&expand=$expand\">Fehlerkorrektur-Datei von einem Datenträger erzeugen</a></td>\n";
echo "<td><a href=\"howtos23.php?way=2&expand=$expand\"><img src=\"../images/good-image.png\" border=\"0\"></a></td>\n";
echo "<td><a href=\"howtos23.php?way=2&expand=$expand\">Fehlerkorrektur-Datei von einem ISO-Abbild erzeugen</a></td>\n";
?>

</tr>
</table>

<?php
if($way==1){
?>
<hr><p>

<table>
<tr>
<td width="200px" align="center"><img src="../images/slot-in.png">
<br><img src="../images/down-arrow.png"></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Legen Sie den Datenträger in ein Laufwerk</b>, 
das direkt mit Ihrem Rechner verbunden ist. Sie können keine Netzwerklaufwerke und keine
Laufwerke innerhalb von virtuellen Maschinen verwenden.</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center"><img src="../images/winbrowser.png">
<br><img src="../images/down-arrow.png"></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Schließen Sie alle Fenster,</b> die Ihr Betriebssystem
möglicherweise öffnet, um den Inhalt des Datenträgers anzuzeigen oder abzuspielen.
Warten Sie mit dem Test, bis das Laufwerk den Datenträger erkannt hat und zur
Ruhe gekommen ist, also z.B. den Datenträger nicht mehr dreht.</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center"><a href="howtosa1.php">
<img src="../images/select-drive.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Wählen Sie in dvdisaster das Laufwerk aus,</b>
in das Sie den Datenträger eingelegt haben.</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center">
<a href="howtosa2.php">
<img src="../images/select-image.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Wählen Sie ein Verzeichnis und einen Dateinamen aus,</b> unter dem Sie das ISO-Abbild des
Datenträgers speichern möchten.</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center">
<a href="howtosa4.php">
<img src="images/read-icon.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Erstellen Sie ein ISO-Abbild</b> des Datenträgers durch
Klicken auf den "Lesen"-Knopf.</td>
</tr>
</table>

<?php begin_howto_shot("Abbild einlesen.","watch-read1.png", "down-arrow.png"); ?>
<b>Beobachten Sie den Fortschritt des Lesevorgangs.</b>
Warten Sie, bis das Abbild vollständig eingelesen wurde. Wenn der Datenträger
aufgrund von Defekten nicht vollständig lesbar ist, 
können Sie keine Fehlerkorrektur-Datei mehr erzeugen.
<?php end_howto_shot(); 
 }  /* end of if($way == 1) */

if($way == 2) {
?>
<hr><p>

<table>
<tr>
<td width="200px" align="center">
<a href="howtosa2.php">
<img src="../images/select-image.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Geben Sie das Verzeichnis und den Dateinamen 
des ISO-Abbilds</b> an, von dem
Sie die Fehlerkorrektur-Datei erstellen möchten.
(In diesem Fall wird davon ausgegangen, daß Sie das
ISO-Abbild schon auf einem anderen Weg erzeugt haben, zum
Beispiel durch Ihre Brennsoftware.)</td>
</tr>
</table>
<?php
}

if($way != 0) {
?>
<table>
<tr>
<td width="200px" align="center">
<a href="howtosa3.php">
<img src="../images/select-ecc.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Wählen Sie ein Verzeichnis und einen Dateinamen aus,</b> 
unter dem Sie die Fehlerkorrektur-Datei speichern möchten.</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center">
<a href="howtosa4.php">
<img src="images/create-icon.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Erstellen Sie die Fehlerkorrektur-Datei</b> durch
Klicken auf den "Erzeugen"-Knopf.</td>
</tr>
</table>

<?php begin_howto_shot("Fehlerkorrektur-Datei erzeugen.","watch-create.png", "down-fork-arrow.png"); ?>
<b>Beobachten Sie den Fortschritt des Vorgangs.</b>
Je nach Größe des Abbilds und der gewählten Redundanz kann die Erstellung der
Fehlerkorrektur-Datei eine Weile dauern.
Für ein 4GB großes DVD-Abbild mit der Redundanz-Einstellung "normal" 
müssen Sie auf einem aktuellen Rechner ca. 5 Minuten einplanen.
<?php end_howto_shot(); ?>

<table>
<tr>
<td width="200px"align="center">
<img src="../images/old-image.png" border="0" align="center">
&nbsp;&nbsp;&nbsp;
<img src="../images/ecc.png" border="0" align="center"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Nachbearbeitung.</b> Sofern Sie das ISO-Abbild nicht noch für 
andere Zwecke benötigen, können Sie es jetzt löschen. Die Fehlerkorrektur-Datei müssen
Sie hingegen gut aufheben und vor Beschädigung schützen. Auf der nächsten Seite gibt
es ein paar Vorschläge, wie Sie die <a href="howtos24.php">Fehlerkorrektur-Datei archivieren</a> können.
</td>
</tr>
</table>

<p>
<a href="howtos24.php">Fehlerkorrektur-Datei archivieren...</a>
<?php
} /* end of if($way != 0) */
?>
<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
