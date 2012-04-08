<?php
# dvdisaster: English homepage translation
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

<h3 class="top">Správné použití dvdisaster</h3>Nyní si ukážeme, jak dvdisaster používá Jane.<p><table width="100%">
<tr>
<td class="w15p">10. Úno. 2004</td>
<td class="w65x"><img src="../images/good-cd.png" alt="Ikona: Nepoškozený disk (bez chyb)"></td>
<td class="w65x"></td>
<td>Jane vytvořila nové CD s důležitými daty.</td>
</tr>
<tr><td colspan="4"><p></td></tr>
<tr>
<td></td>
<td><img class="valignt" src="../images/good-cd.png" alt="Ikona: Nepoškozený disk (bez chyb)"></td>
<td><img class="valignt" src="../images/ecc.png" alt="Ikona: Samostatný soubor s daty pro opravu chyb"></td>
<td>Aby CD ochránila před ztrátou dat <a href="howtos20.php">vytvořila pomocí dvdisaster data pro opravu chyb</a>. Oboje uchovala pro pozdější použití.</td>
</tr>
<tr><td colspan="4"><hr> </td></tr>
<tr>
<td>14. Kvě. 2005</td>
<td><img class="valignt" src="../images/good-cd.png" alt="Ikona: Nepoškozený disk (bez chyb)"></td>
<td><img class="valignt" src="../images/ecc.png" alt="Ikona: Samostatný soubor s daty pro opravu chyb"></td>
<td>Jane ví, že při běžném užívání není běžně přistupováno ke všem datům na CD. Proto po uplynutí jednoho roku <a href="howtos10.php">provede kontrolu čitelnosti CD</a> aby se ujistila, že v některé z málo používaných oblastí nedošlo k poškození. CD je však po roce stále nepoškozené.</td>
</tr>
<tr><td colspan="4"><hr> </td></tr>
<tr>
<td>19. Srp 2007</td>
<td><img class="valignt" src="../images/bad-cd.png" alt="Ikona: Poškozený disk (částečně nečitelný)"></td>
<td><img class="valignt" src="../images/ecc.png" alt="Ikona: Samostatný soubor s daty pro opravu chyb"></td>
<td>Uplynuly další dva roky a Jane si všimla, že některá z dat na CD již nejsou čitelná. <a href="howtos10.php">Kontrola čitelnosti</a> potvrdila, že CD je poškozené.</td>
</tr>
<tr>
 <td align="right"><a href="howtos42.php#a">načtení</a></td>
 <td align="center"><img class="valignt" src="../images/down-arrow.png" alt="Ikona: Šipka dolů"></td>
 <td></td><td></td>
</tr>
<tr><td colspan="4"><p></td></tr>
<tr>
<td></td>
<td><img class="valignt" src="../images/bad-image.png" alt="Ikona: Neúplná bitová kopie"></td>
<td><img class="valignt" src="../images/ecc.png" alt="Ikona: Samostatný soubor s daty pro opravu chyb"></td>
<td>Jane použije dvdisaster pro <a href="howtos42.php#a">načtení maximálního možného počtu sektorů</a> z poškozeného CD do bitové kopie ve formátu ISO.</td>
<tr>
 <td align="right"><a href="howtos42.php#b">oprava</a></td>
 <td align="center" colspan="2"><img class="valignt" src="../images/dbl-arrow-left.png" alt="Ikona: Dvojitá šipka vlevo"></td>
 <td></td>
</tr>
<tr><td colspan="4"><p></td></tr>
<tr>
<td></td>
<td><img class="valignt" src="../images/good-image.png" alt="Ikona: Kompletní bitová kopie"></td>
<td><img class="valignt" src="../images/ecc.png" alt="Ikona: Samostatný soubor s daty pro opravu chyb"></td>
<td>Za pomoci dat pro opravu chyb Jane <a href="howtos42.php#b">obnoví chybějící části bitové kopie</a>.<tr>
 <td align="right">Zapsání nového CD</td>
 <td align="center"><img class="valignt" src="../images/down-arrow.png" alt="Ikona: Šipka dolů"></td>
 <td></td><td></td>
</tr>
<tr><td colspan="4"><p></td></tr>
<tr>
<td></td>
<td><img class="valignt" src="../images/good-cd.png" alt="Ikona: Nepoškozený disk (bez chyb)"></td>
<td><img class="valignt" src="../images/ecc.png" alt="Ikona: Samostatný soubor s daty pro opravu chyb"></td>
<td>Jane vypálí opravenou bitovou kopii ISO na CD. Data pro opravu chyb si stále ponechá, pro případ že někdy v budoucnu dojde k poškození nově vypáleného CD.</td>
</table>


<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
