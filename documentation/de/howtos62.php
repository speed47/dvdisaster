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

<h3 class="top">Richtige Anwendung von dvdisaster</h3>

Das folgende Beispiel skizziert, wie Jane dvdisaster anwendet. <p>

<table width="100%">
<tr>
<td class="w15p">10. Feb. 2004</td>
<td class="w65x"><img src="../images/good-cd.png" alt="Symbol: Guter Datenträger (ohne Lesefehler)"></td>
<td class="w65x"></td>
<td>Jane brennt eine neue CD mit wichtigen Daten.</td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td> </td>
<td><img class="valignt" src="../images/good-cd.png" alt="Symbol: Guter Datenträger (ohne Lesefehler)"></td>
<td><img class="valignt" src="../images/ecc.png" alt="Symbol: Eigenständige Fehlerkorrektur-Datei"></td>
<td>Um ihre CD vor Datenverlust zu schützen, 
    <a href="howtos20.php">erzeugt sie mit dvdisaster Fehlerkorrektur-Daten</a>.<br>
    Beides hebt sie für eine zukünftige Verwendung auf.</td>
</tr>
<tr><td colspan="4"> <hr> </td></tr>
<tr>
<td>14. Mai 2005</td>
<td><img class="valignt" src="../images/good-cd.png" alt="Symbol: Guter Datenträger (ohne Lesefehler)"></td>
<td><img class="valignt" src="../images/ecc.png" alt="Symbol: Eigenständige Fehlerkorrektur-Datei"></td>
<td>Jane weiß, daß bei einer normalen Nutzung Ihrer CD nicht auf alle Datenbereiche
   täglich zugegriffen wird. Daher führt sie nach einem Jahr mit dvdisaster eine
   <a href="howtos10.php">Überprüfung auf Lesefehler</a> durch, um sicherzustellen,
   daß die CD nicht an einer wenig benutzten Stelle bereits Beschädigungen aufweist.
   Nach gut einem Jahr ist die CD noch völlig in Ordnung.</td>
</tr>
<tr><td colspan="4"> <hr> </td></tr>
<tr>
<td>19. Aug 2007</td>
<td><img class="valignt" src="../images/bad-cd.png" alt="Symbol: Beschädigter Datenträger (teilweise unlesbar)"></td>
<td><img class="valignt" src="../images/ecc.png" alt="Symbol: Eigenständige Fehlerkorrektur-Datei"></td>
<td>Nach weiteren zwei Jahren stellt Jane fest,
daß sie einige Dateien auf der CD nicht mehr lesen kann. 
Eine <a href="howtos10.php">Überprüfung auf Lesefehler</a> bestätigt, daß
die CD durch Alterung unbrauchbar geworden ist und defekte Sektoren aufweist.</td>
</tr>
<tr>
 <td align="right"><a href="howtos42.php#a">lesen</a></td>
 <td align="center"><img class="valignt" src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten"></td>
 <td></td><td></td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td> </td>
<td><img class="valignt" src="../images/bad-image.png" alt="Symbol: Unvollständiges Abbild"></td>
<td><img class="valignt" src="../images/ecc.png" alt="Symbol: Eigenständige Fehlerkorrektur-Datei"></td>
<td>Jane verwendet dvdisaster, um von der beschädigten CD 
<a href="howtos42.php#a">noch so viele Sektoren wie möglich in ein ISO-Abbild</a>
einzulesen.</td>
<tr>
 <td align="right"><a href="howtos42.php#b">wieder-<br>herstellen</a></td>
 <td align="center" colspan="2"><img class="valignt" src="../images/dbl-arrow-left.png" alt="Symbol: Doppelter Pfeil nach links"></td>
 <td></td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td> </td>
<td><img class="valignt" src="../images/good-image.png" alt="Symbol: Vollständiges Abbild"></td>
<td><img class="valignt" src="../images/ecc.png" alt="Symbol: Eigenständige Fehlerkorrektur-Datei"></td>
<td>Mit Hilfe der Fehlerkorrektur-Daten kann Jane das
    <a href="howtos42.php#b">ISO-Abbild vollständig wiederherstellen</a>.
<tr>
 <td align="right">Neue CD brennen</td>
 <td align="center"><img class="valignt" src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten"></td>
 <td></td><td></td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td> </td>
<td><img class="valignt" src="../images/good-cd.png" alt="Symbol: Guter Datenträger (ohne Lesefehler)"></td>
<td><img class="valignt" src="../images/ecc.png" alt="Symbol: Eigenständige Fehlerkorrektur-Datei"></td>
<td>Mit dem wiederhergestellten ISO-Abbild brennt Jane eine neue CD.
    Die Fehlerkorrektur-Daten behält sie für den Fall, daß auch die neue CD
kaputt geht.</td>
</table>


<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
