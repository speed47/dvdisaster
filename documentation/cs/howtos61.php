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

<h3 class="top">Podstata opravy chyb</h3>

<table width="100%">
<tr valign="top">
<td><img src="../images/bad-cd.png" alt="Ikona: Poškozený disk (částečně nečitelný)"></td>
<td></td>
<td><img src="../images/ecc.png" alt="Ikona: Samostatný soubor s daty pro opravu chyb"></td>
<td> </td>
<td rowspan="3">Příklad z předchozí stránky ukázal, jak dvdisaster obnovuje data za pomoci kombinace čitelných částí disku a dat pro opravu chyb.<p>Pro maximální využití dvdisaster je užitečné znát alespoň základy použité metody opravy chyb. A když už jsme u toho, pojďme vyvrátit občas slýchávaný omyl - data pro opravu chyb <b>nejsou</b> jen kopie posledních 20% datových sektorů. To by byl opravdu laciný vtip ;-)</td>
</tr>

<tr>
<td align="right" class="w65x">80%<img src="../images/rdiag-arrow.png" alt="Ikona: Šipka šikmo doprava"></td>
<td></td>
<td align="left" class="w65x"><img src="../images/ldiag-arrow.png" alt="Ikona: Šipka šikmo doleva">20%</td>
<td></td>
</tr>

<tr>
<td></td>
<td><img src="../images/good-image.png" alt="Ikona: Kompletní bitová kopie"></td>
<td></td>
<td></td>
</tr>
</table><p><b>Příklad: PIN k Anninu šuplíku</b><p>Anna má stůl, jehož šuplíky lze otevřít jen po odemčení kódového klíče zadáním kombinace &quot;8 6 2 3&quot;. Protože šuplíky neobsahují žádné důvěrné informace, rozhodne se poznamenat si kombinaci přímo na desku stolu:<p><img src="../images/ecc-example1.png" alt="8 6 2 3"><p>Anna je opatrná a předpokládá proto, že některé z čísel se může stát nečitelné (například že na něj z neopatrnosti vylije inkoust). Poznamená si proto také součet všech čtyř čísel (znaky &quot;+&quot; a &quot;=&quot; byly přidány pouze pro názornost):<p><img src="../images/ecc-example2.png" alt="8+6+2+3=19"><p>Po čase je jedno z čísel opravdu zakryto inkoustovou skvrnou a stane se nečitelné:<p><img src="../images/ecc-example3.png" alt="8+ +6+2+3=19"><p>To ale není problém, Anna může chybějící číslo <i>x</i> dopočítat za pomoci zbývajících částí vzorce:<p>8 + x + 2 + 3 = 19, takže<p>x = 19 - 8 - 2 - 3, a proto x = 6.<p>Je názorně vidět, že libovolné z daných pěti čísel může být s použitím zbývajících čtyř kdykoliv obnoveno. Příklad také demonstruje některé z důležitých vlastností opravy chyb:<p><table><tr><td><img src="../images/ecc-example4.png" alt="8+6+2+3 (disk)=19 (ECC)"></td><td>  </td>
<td class="valignt">Pro danou sadu dat (např. čísla &quot;8 6 2 3&quot;) mohou být vytvořena data pro opravu chyb (tedy např. součet &quot;19&quot;) s jejichž pomocí lze chybějící údaje dopočítat z údajů zbývajících.<p>Stejný princip využívá dvdisaster; chráněný řetězec číslic není nic jiného než bitová kopie CD, DVD nebo BD ve formátu ISO.</td>
</tr></table><p>Koncept <b>redundance</b> může být vysvětlen následovně:<ul>
<li>Pro čtyři vstupní čísla je vypočítáno jedno &quot;číslo pro opravu chyb&quot;. 1 ze 4 (nebo 1/4) představuje redundanci 25%.</li>
<li>Z jednoho čísla pro opravu chyb můžeme dopočítat přesně jedno chybějící číslo, neboli maximálně 25% dat. Redundance odpovídá maximální kapacitě opravy chyb.</li>
<li>Dodatečný prostor vyžadovaný daty pro opravu chyb je také odvozen od redundance (v tomto příkladu 25%).</li>
</ul>dvdisaster využívá termín redundance v odpovídajícím smyslu. Také si všimněte, že<ul>
<li>nelze obnovit žádná data, pokud ztráta dat přesáhne použitou redundanci (vzorec v příkladu nelze vyřešit pro dvě a více neznámých).</li>
<li>data pro opravu chyb musí být vytvořena v době, kdy jsou ještě všechna data čitelná.</li>
</ul><p>Výše uvedený příklad nezobecňuje schéma pro opravu chyb pro obnovu více než jednoho údaje. V takovém případě je nutné použít mnohem složitější výpočetní systém umožňující řešení pro více než jednu chybějící hodnotu. dvdisaster používá Reed-Solomon kódování, které má přesně tyto vlastnosti, matematika potřebná k jeho řešení se ale ve škole neučí. Uživatelé s větším zájmem o tématiku mohou potřebné informace nalézt v některé z knih o teorii šifrování.<!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>