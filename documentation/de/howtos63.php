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

<h3>Falsche Anwendung von dvdisaster</h3>

Joe verläßt sich darauf, daß seine Datenträger 
auch ohne zusätzlichen Schutz halten.<p>

<table width="100%">
<tr>
<td width="15%">10. Feb. 2004</td>
<td width="60px"><img src="../images/good-cd.png"></td>
<td width="60px"><img src="../images/good-cd.png"></td>
<td>Joe brennt zwei CDs mit wichtigen Daten. Er trifft jedoch keine weiteren
Maßnahmen, um die Daten auf seinen CDs zu schützen.</td>
</tr>
<tr><td colspan="4"> <hr> </td></tr>
<tr>
<td>14. Mai 2005</td>
<td><img align="top" src="../images/good-cd.png"></td>
<td><img align="top" src="../images/good-cd.png"></td>
<td>Joe verwendet seine CDs regelmäßig. Sie sind nach einem Jahr
noch völlig in Ordnung.</td>
</tr>
<tr><td colspan="4"> <hr> </td></tr>
<tr>
<td>19. Aug 2007</td>
<td><img align="top" src="../images/bad-cd.png"></td>
<td><img align="top" src="../images/good-cd.png"></td>
<td>Nach weiteren zwei Jahren stellt Joe fest, daß er auf einige Daten von seiner
ersten CD nicht mehr zugreifen kann.</td> 
</tr>
<tr>
 <td align="right"><a href="howtos10.php">prüfen</a></td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td></td>
</tr>
<tr>
<td>20. Aug 2007</td>
<td><img align="top" src="../images/bad-cd.png"></td>
<td><img align="top" src="../images/bad-cd.png"></td>
<td>Joe lädt sich dvdisaster herunter und führt eine 
<a href="howtos10.php">Überprüfung auf Lesefehler</a> durch. 
Dabei stellt er fest, daß die CD 25000 unlesbare Sektoren enthält.
Ein Test der zweiten CD ergibt, daß auch diese bisher 
unbemerkt 1500 unlesbare Sektoren entwickelt hat. </td>
</tr>
<tr>
 <td align="right">lesen</td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td></td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td>21. Aug 2007</td>
<td><img align="top" src="../images/bad-image.png"></td>
<td><img align="top" src="../images/bad-image.png"></td>
<td>Joe verwendet dvdisaster, um von den beschädigten CDs noch 
so viele Sektoren wie möglich einzulesen. 
Da er jedoch keine Fehlerkorrektur-Daten hat, kann er
die verbleibenden unlesbaren Sektoren nicht neu durch dvdisaster berechnen lassen.</td>
</tr>
<tr>
 <td align="right">viele Lese-<br>versuche</td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td></td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td>05. Sep 2007</td>
<td><img align="top" src="../images/bad-image.png"></td>
<td><img align="top" src="../images/good-image.png"></td>
<td>Joe nutzt aus, daß dvdisaster ein unvollständiges Abbild durch mehrere
Leseversuche weiter vervollständigen kann. Er überträgt die beschädigten Abbilder
nacheinander auf mehrere Rechner, um Leseversuche mit verschiedenen Laufwerken
zu unternehmen. Nach gut zwei Wochen konnte er zumindest die fehlenden Sektoren
der zweiten CD wieder einlesen. Von der ersten CD sind jedoch noch immer 21000 
Sektoren unlesbar.</td>
</tr>
<tr>
 <td align="right">nur eine CD<br>gerettet</td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td></td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td>06. Sep 2007</td>
<td><img align="top" src="../images/bad-cd.png"></td>
<td><img align="top" src="../images/good-cd.png"></td>
<td>Joe schreibt die erste CD als unrettbar ab und freut sich, daß er zumindest
von dem Abbild der zweiten CD wieder einen neuen Datenträger brennen kann. 
Wenn er rechtzeitig Fehlerkorrektur-Daten erzeugt hätte, wären ihm 
vermutlich<sup>1)</sup> die zwei Wochen Leseversuche erspart geblieben und er hätte
die Daten von beiden CDs noch retten können.</td></tr>
</table>
<hr>
<sup>1)</sup>Die Fehlerkorrektur setzt einen typischen Alterungsprozeß voraus.
Wenn die CD total zerstört wird, kann sie auch mit Hilfe der Fehlerkorrektur-Daten
nicht mehr gerettet werden. Für sehr wichtige Daten müssen Sie zusätzliche 
Schutzmaßnahmen treffen, indem Sie zum Beispiel mehrere Kopien auf unterschiedlichen
Datenträger-Typen erstellen.

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
