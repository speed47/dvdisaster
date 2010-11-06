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

<h3>Richtige Anwendung von dvdisaster</h3>

Das folgende Beispiel skizziert, wie Jane dvdisaster anwendet. <p>

<table width="100%">
<tr>
<td width="15%">10. Feb. 2004</td>
<td width="60px"><img src="../images/good-cd.png"></td>
<td width="60px"></td>
<td>Jane brennt eine neue CD mit wichtigen Daten.</td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td> </td>
<td><img align="top" src="../images/good-cd.png"></td>
<td><img align="top" src="../images/ecc.png"></td>
<td>Um ihre CD vor Datenverlust zu schützen, 
    <a href="howtos20.php">erzeugt sie mit dvdisaster Fehlerkorrektur-Daten</a>.<br>
    Beides hebt sie für eine zukünftige Verwendung auf.</td>
</tr>
<tr><td colspan="4"> <hr> </td></tr>
<tr>
<td>14. Mai 2005</td>
<td><img align="top" src="../images/good-cd.png"></td>
<td><img align="top" src="../images/ecc.png"></td>
<td>Jane weiß, daß bei einer normalen Nutzung Ihrer CD nicht auf alle Datenbereiche
   täglich zugegriffen wird. Daher führt sie nach einem Jahr mit dvdisaster eine
   <a href="howtos10.php">Überprüfung auf Lesefehler</a> durch, um sicherzustellen,
   daß die CD nicht an einer wenig benutzten Stelle bereits Beschädigungen aufweist.
   Nach gut einem Jahr ist die CD noch völlig in Ordnung.</td>
</tr>
<tr><td colspan="4"> <hr> </td></tr>
<tr>
<td>19. Aug 2007</td>
<td><img align="top" src="../images/bad-cd.png"></td>
<td><img align="top" src="../images/ecc.png"></td>
<td>Nach weiteren zwei Jahren stellt Jane fest,
daß sie einige Dateien auf der CD nicht mehr lesen kann. 
Eine <a href="howtos10.php">Überprüfung auf Lesefehler</a> bestätigt, daß
die CD durch Alterung unbrauchbar geworden ist und defekte Sektoren aufweist.</td>
</tr>
<tr>
 <td align="right"><a href="howtos42.php#a">lesen</a></td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td></td><td></td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td> </td>
<td><img align="top" src="../images/bad-image.png"></td>
<td><img align="top" src="../images/ecc.png"></td>
<td>Jane verwendet dvdisaster, um von der beschädigten CD 
<a href="howtos42.php#a">noch so viele Sektoren wie möglich in ein ISO-Abbild</a>
einzulesen.</td>
<tr>
 <td align="right"><a href="howtos42.php#b">wieder-<br>herstellen</a></td>
 <td align="center" colspan="2"><img align="top" src="../images/dbl-arrow-left.png"></td>
 <td></td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td> </td>
<td><img align="top" src="../images/good-image.png"></td>
<td><img align="top" src="../images/ecc.png"></td>
<td>Mit Hilfe der Fehlerkorrektur-Daten kann Jane das
    <a href="howtos42.php#b">ISO-Abbild vollständig wiederherstellen</a>.
<tr>
 <td align="right">Neue CD brennen</td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td></td><td></td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td> </td>
<td><img align="top" src="../images/good-cd.png"></td>
<td><img align="top" src="../images/ecc.png"></td>
<td>Mit dem wiederhergestellten ISO-Abbild brennt Jane eine neue CD.
    Die Fehlerkorrektur-Daten behält sie für den Fall, daß auch die neue CD
kaputt geht.</td>
</table>


<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
