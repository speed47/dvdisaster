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

<h3 class="top">Chybné použití dvdisaster</h3>Joe se spoléhá na to, že jeho disky si svůj obsah udrží i bez dodatečné ochrany.<p><table width="100%">
<tr>
<td class="w15p">10. Úno. 2004</td>
<td class="w65x"><img src="../images/good-cd.png" alt="Ikona: Nepoškozený disk (bez chyb)"></td>
<td class="w65x"><img src="../images/good-cd.png" alt="Ikona: Nepoškozený disk (bez chyb)"></td>
<td>Joe vytvoří dva CD disky obsahující důležitá data. Nevytvoří ale žádnou zálohu pro případ ztráty dat.</td>
</tr>
<tr><td colspan="4"><hr> </td></tr>
<tr>
<td>14. Kvě. 2005</td>
<td><img class="valignt" src="../images/good-cd.png" alt="Ikona: Nepoškozený disk (bez chyb)"></td>
<td><img class="valignt" src="../images/good-cd.png" alt="Ikona: Nepoškozený disk (bez chyb)"></td>
<td>Joe svá CD pravidelně používá. Po jednom roce jsou stále bez problémů čitelná.</td>
</tr>
<tr><td colspan="4"><hr> </td></tr>
<tr>
<td>19. Srp 2007</td>
<td><img class="valignt" src="../images/bad-cd.png" alt="Ikona: Poškozený disk (částečně nečitelný)"></td>
<td><img class="valignt" src="../images/good-cd.png" alt="Ikona: Nepoškozený disk (bez chyb)"></td>
<td>Po uplynutí dalších dvou let Joe zjišťuje, že některá data na jednom z CD již nejsou čitelná.</td> 
</tr>
<tr>
 <td align="right"><a href="howtos10.php">kontrola</a></td>
 <td align="center"><img class="valignt" src="../images/down-arrow.png" alt="Ikona: Šipka dolů"></td>
 <td align="center"><img class="valignt" src="../images/down-arrow.png" alt="Ikona: Šipka dolů"></td>
 <td></td>
</tr>
<tr>
<td>20. Srp 2007</td>
<td><img class="valignt" src="../images/bad-cd.png" alt="Ikona: Poškozený disk (částečně nečitelný)"></td>
<td><img class="valignt" src="../images/bad-cd.png" alt="Ikona: Poškozený disk (částečně nečitelný)"></td>
<td>Joe stáhne dvdisaster a provede <a href="howtos10.php">kontrolu čitelnosti</a>. Zjistí, že CD obsahuje 25000 nečitelných sektorů. Při kontrole druhého CD zjistí, že i to má 1500 nečitelných sektorů, kterých si zatím nevšiml.</td>
</tr>
<tr>
 <td align="right">načtení</td>
 <td align="center"><img class="valignt" src="../images/down-arrow.png" alt="Ikona: Šipka dolů"></td>
 <td align="center"><img class="valignt" src="../images/down-arrow.png" alt="Ikona: Šipka dolů"></td>
 <td></td>
</tr>
<tr><td colspan="4"><p></td></tr>
<tr>
<td>21. Srp 2007</td>
<td><img class="valignt" src="../images/bad-image.png" alt="Ikona: Neúplná bitová kopie"></td>
<td><img class="valignt" src="../images/bad-image.png" alt="Ikona: Neúplná bitová kopie"></td>
<td>Joe použije dvdisaster pro načtení co největšího počtu sektorů poškozeného disku. Protože ale nemá žádná data pro opravu chyb, neexistuje žádný způsob, jak nečitelné sektory dopočítat.</td>
</tr>
<tr>
 <td align="right">dodatečné pokusy<br>o načtení</td>
 <td align="center"><img class="valignt" src="../images/down-arrow.png" alt="Ikona: Šipka dolů"></td>
 <td align="center"><img class="valignt" src="../images/down-arrow.png" alt="Ikona: Šipka dolů"></td>
 <td></td>
</tr>
<tr><td colspan="4"><p></td></tr>
<tr>
<td>05. Zář 2007</td>
<td><img class="valignt" src="../images/bad-image.png" alt="Ikona: Neúplná bitová kopie"></td>
<td><img class="valignt" src="../images/good-image.png" alt="Ikona: Kompletní bitová kopie"></td>
<td>Joe využije funkce dvdisaster pro doplnění bitové kopie pomocí vícenásobného čtení. Přenáší poškozenou bitovou kopii mezi počítači a pokouší se o čtení pomocí různých mechanik. Po dvou týdnech zkoušení se mu podařilo načíst alespoň všechny sektory druhého CD. Na prvním CD však stále zbývá 21000 nečitelných sektorů.</td>
</tr>
<tr>
 <td align="right">opraveno jen<br>jedno CD</td>
 <td align="center"><img class="valignt" src="../images/down-arrow.png" alt="Ikona: Šipka dolů"></td>
 <td align="center"><img class="valignt" src="../images/down-arrow.png" alt="Ikona: Šipka dolů"></td>
 <td></td>
</tr>
<tr><td colspan="4"><p></td></tr>
<tr>
<td>06. Zář 2007</td>
<td><img class="valignt" src="../images/bad-cd.png" alt="Ikona: Poškozený disk (částečně nečitelný)"></td>
<td><img class="valignt" src="../images/good-cd.png" alt="Ikona: Nepoškozený disk (bez chyb)"></td>
<td>Joe usoudí, že první CD je neopravitelné a považuje za štěstí, že se mu podařilo získat kompletní bitovou kopii druhého CD. Pokud by však včas vytvořil data pro opravu chyb, ušetřil by pravděpodobně <sup>1)</sup> dva týdny pokusů o přečtení a obnovil by obě CD.</td></tr>
</table>
<hr><sup>1)</sup>Oprava chyb předpokládá běžný proces stárnutí. Pokud je CD vážně poškozeno, nelze ho opravit ani s pomocí dat pro opravu chyb. Při ochraně dat nespoléhejte pouze na dvdisaster samotný; použijte i další opatření, jako je vytvoření záložních kopií na rozdílném druhu datových nosičů.<!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>