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

howto_headline("Datenträger überprüfen", "Übersicht", "images/scan-icon.png");
?>

<!--- Insert actual page content below --->

<table width="100%" cellspacing="5">
<tr>
 <td><b>Aufgabe</b></td>
 <td>
   Der Datenträger wird auf beschädigte Sektoren überprüft.
  </td>
</tr>
<tr><td> <pre> </pre> </td></tr>

<tr>
 <td colspan="2"><b>Benötigt werden:</b></td>
</tr>
<tr>
 <td width="150px"><img src="../images/good-cd.png" align="top">
   &nbsp; <img src="../images/bad-cd.png" align="top"></td>
<td>
  Ein Datenträger in beliebigem Zustand (gut oder bereits mit Lesefehlern).
</td>
</tr>

<tr>
 <td><img src="../images/ecc.png"></td>
 <td>Wenn Sie Fehlerkorrektur-Daten haben, werden bei der Überprüfung einige zusätzliche
  Tests durchgeführt. Es geht aber auch ohne Fehlerkorrektur-Daten.
 </td>
</tr>

<tr><td> <pre> </pre> </td></tr>

<tr valign="top">
 <td><b>Dies ist zu tun:</b></td>
 <td>
  1. <a href="howtos11.php">Grundeinstellungen vornehmen</a><br>
  2. <a href="howtos12.php">Datenträger überprüfen</a><br>
  3. <a href="howtos13.php">Ergebnisse bewerten</a><br>
 </td>
</tr>

<tr><td> <pre> </pre> </td></tr>

<tr valign="top">
 <td><b>Verwandte Funktionen:<p></td>
 <td><a href="howtos30.html">Einlesen von beschädigten Datenträgern</a> und
     <a href="howtos40.html">Wiederherstellen von Abbildern</a>.</td>
 </tr>
</table><p>

<pre> </pre>

<a href="howtos11.php">Grundeinstellungen vornehmen...</a>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
