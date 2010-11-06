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

<h3>Bedienelemente</h3>

Dieses Kapitel erklärt einige der häufig verwendeten Bedienelemente:

<pre> </pre>

<table width="100%">
<tr>
<td align="center"><a href="howtosa1.php"><img src="../images/good-cd.png" border="0"></a></td>
<td>Das <a href="howtosa1.php">Menü zum Auswählen von Laufwerken</a>.</td>
</tr>
<tr>
<tr><td>&nbsp;</td><td></td></tr>
<td align="center"><a href="howtosa2.php"><img src="../images/good-image.png" border="0"></a></td>
<td>Das <a href="howtosa2.php">Fenster zum Auswählen von Abbild-Dateien</a>.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>
<tr>
<td align="center"><a href="howtosa3.php"><img src="../images/ecc.png" border="0"></a></td>
<td>Das <a href="howtosa3.php">Fenster zum Auswählen von Fehlerkorrektur-Dateien</a>.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>
<tr>
<td align="center"><a href="howtosa4.php">
  <img src="images/read-icon.png" border="0">
  <img src="images/create-icon.png" border="0"><br>
  <img src="images/scan-icon.png" border="0">
  <img src="images/fix-icon.png" border="0"><br>
  <img src="images/compare-icon.png" border="0">
  <img src="images/stop-icon.png" border="0">
</a></td>
<td>Die <a href="howtosa4.php">Piktogramme zum Auswählen von Aktionen</a>.</td>
</tr>
</table>



<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
