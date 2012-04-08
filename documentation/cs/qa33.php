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

<h3 class="top">Metody RS01, RS02 a RS03</h3>dvdisaster nabízí tři metody opravy chyb. RS01 a RS02 jsou současné a vyzkoušené metody, RS03 je stále ještě vyvíjena.<p><b>Porovnání metod.</b> Všechny metody používají opravu chyb <a href="qa31.php">Reed-Solomon</a>. Počítají informace pro opravu chyb pro bitové kopie ISO, které pak slouží k opravě nečitelných sektorů, pokud dojde k poškození disku.<p>Metody se liší způsobem ukládání informací pro opravu chyb:<p><ul>
<li><a name="file"> </a>RS01 vytváří <b>soubory pro opravu chyb</b>, které jsou ukládány odděleně od bitové kopie ke které náleží. Protože ochrana dat na <a href="qa32.php#file">úrovni souborů</a> je obtížná, musí být soubory pro opravu chyb uloženy na disku, který je také chráněn prostřednictvím dvdisaster.<p></li>

<li><a name="image"> </a>Pro aplikaci metody RS02 je nejdříve na pevném disku s pomocí programu pro vypalování CD/DVD vytvořena bitová kopie. Před vypálením na disk je tato bitová kopie pomocí dvdisaster <b>rozšířena</b> o data pro opravu chyb. Chráněná data a informace pro opravu chyb jsou tak uloženy na stejném disku. Poškozené sektory v oblasti obsahující informace pro opravu chyb snižují kapacitu opravy chyb, ale opravu neznemožňují - není vyžadován dodatečný disk pro uchovávání nebo ochranu informací pro opravu.<p></li>
</ul>RS03 je vylepšením verzí RS01 a RS02. S jeho pomocí lze vytvářet jak soubory pro opravu chyb, tak rozšířené bitové kopie:<ul>
<li>RS03 umí práci rozložit na několik jader procesoru a na moderních procesorech je tak mnohem rychlejší než RS01/RS02.</li>
<li>RS03 <b>soubory pro opravu chyb</b> jsou - na rozdíl od RS01 - odolné vůči poškození. To by vás však nemělo svádět k nedbalému zacházení s vašimi soubory pro opravu chyb - nevýhody <a href="qa32.php#file">čtení na úrovni souborového systému</a> jsou stále platné.</li>
<li>RS03 <b>rozšířené bitové kopie</b> nevyžadují takzvané hlavní bloky obsahující důležité informace. Díky tomu je RS03 více robustní, ale také více omezující: Rozšířená bitová kopie musí zcela zaplnit cílový disk, kdežto u RS02 bylo možné si zvolit libovolnou velikost rozšířené bitové kopie.</li>
</ul>Změny umožňující paralelizaci výpočtů a zvýšení robustnosti způsobily snížení prostorové efektivnosti RS03, RS03 data pro opravu chyb tak mají o něco nižší kapacitu opravy chyb, než jejich stejně velké ekvivalenty používající RS01/RS02.<p><a name="table"> </a> <b>Porovnání způsobu uložení opravy chyb.</b><p>Následující tabulky shrnuje rozdíly mezi soubory pro opravu chyb (RS01, RS03) a rozšířenými bitovými kopiemi (RS02, RS03):<p><table width="100%" border="1" cellspacing="0" cellpadding="5">
<tr>
<td class="w50p"><i>Soubory pro opravu chyb</i></td>
<td class="w50p"><i>Bitová kopie rozšířená o data pro opravu chyb</i></td>
</tr>
<tr valign="top">
<td>může být zvolena libovolná redundance</td>
<td>redundance je vymezena volným místem na disku<br> (= kapacita disku - velikost dat)</td>
</tr>

<tr valign="top">
<td>efektivní už při 15% redundanci, protože u souborů pro opravu chyb se předpokládá, že nejsou poškozeny</td>
<td>vyžaduje větší redundanci (doporučeno: 20-30%) pro kompenzaci případného poškození dat pro opravu chyb</td> 
</tr>

<tr valign="top">
<td>disk může být zcela zaplněn uživatelskými daty</td>
<td>využitelný prostor na disku je zmenšen o velikost dat pro opravu chyb</td> 
</tr>

<tr valign="top">
<td>mohou být vytvořeny dodatečně pro již existující disk</td>
<td>použitelné pouze při vytváření disku, protože bitová kopie musí být o data pro opravu chyb rozšířena před vypálením</td>
</tr>

<tr valign="top">
<td>uložení dat pro opravu chyb odděleně od dat zvyšuje ochranu dat</td>
<td>uložení data pro opravu chyb společně s uživatelskými daty může snížit kapacitu opravy chyb</td>
</tr>

<tr valign="top">
<td>Musí být veden přehled o příslušnosti dat pro opravu chyb k disku. Soubory pro opravu chyb musí být chráněny proti poškození.</td>
<td>Jednoduché řešení na jednom disku; informace pro opravu chyb není nutné zvlášť evidovat ani chránit.</td></tr>

<tr valign="top">
<td>žádné problémy s kompatibilitou</td>
<td>disky s bitovou kopií rozšířenou o data pro opravu chyb nemusí být v některých mechanikách správně přehrána</td>
</tr>
</table><p><!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>