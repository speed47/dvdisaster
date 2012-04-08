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

<h3 class="top">Celkový obraz - srovnání dvdisaster s běžnou zálohou</h3>dvdisaster ukládá data na CD/DVD/BD tak, že jsou zcela obnovitelná i poté, co se na disku objeví nějaké nečitelné oblasti. Metoda použitá dvdisaster vyžaduje mnohem méně prostoru (nebo dodatečných disků) něž kompletní záloha. Před použitím dvdisaster je třeba znát podobnosti a rozdíly mezi dvdisaster a běžnou (kompletní) zálohou:<p>Nejdříve si zrekapitulujme, jak pracujeme s běžnými zálohami:<p><table width="100%">
<tr>
<td class="w65x"><img src="../images/backup1.png" alt="Ikona: Originální disk"></td>
<td class="w65x">Kopie<br><img src="../images/right-arrow.png" alt="Ikona: Šipka vpravo"></td>
<td class="w65x"><img src="../images/backup2.png" alt="Ikona: Záložní disk"></td>
<td> </td>
<td>Existující disk (1) je zkopírován na záložní disk (2).</td>
</tr>

<tr>
<td align="center"><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů">  </td>
<td></td>
<td align="center"><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů">  </td>
<td></td>
</tr>

<tr>
<td class="w65x"><img src="../images/bad-cd1.png" alt="Ikona: Poškozený disk"></td>
<td class="w65x"></td>
<td class="w65x"><img src="../images/backup2.png" alt="Ikona: Záložní disk"></td>
<td></td>
<td>Pokud je poté některý z těchto dvou disků poškozen, máme ještě nepoškozený záložní disk.</td>
</tr>
</table><p>Existují případy, kdy je důležité mít záložní kopii CD/DVD/BD: jeden z disků se může ztratit, roztrhnout se v mechanice nebo může být zničen špatným používáním. Pokud je však s disky správně zacházeno, jsou tyto případy kompletní ztráty dat vzácné.<p>Mnohem častější je, že disk po pár letech začne postupně ztrácet čitelnost - nevyhnutelný proces stárnutí. Pokud je disk pravidelně používán (nebo kontrolován), bude ztráta dat většinou odhalena poté, co svou čitelnost ztratí přibližně 5% až 10% disku. V tomto okamžiku je disk jako celek nepoužitelný, ale z 90% stále čitelný. <i>Kompletní záložní kopie disku není v tomto případě třeba; stačí pouze nějakým způsobem obnovit oněch 10% ztracených dat.</i><p>To je právě případ pro který byl dvdisaster vytvořen. Uvažte toto:<p><table width="100%">
<tr>
<td class="w65x"><img src="../images/good-cd.png" alt="Ikona: Nepoškozený disk (bez chyb)"></td>
<td class="w65x">Vytvoření<br><img src="../images/right-arrow.png" alt="Ikona: Šipka vpravo"><br>ECC</td>
<td class="w65x"><img src="../images/ecc.png" alt="Ikona: Samostatný soubor s daty pro opravu chyb"></td>
<td> </td>
<td>Tentokrát nevytvoříme kompletní zálohu. Za pomoci dvdisaster vytvoříme data pro opravu chyb (&quot;ECC&quot;) s jejichž pomocí lze obnovit až 20% poškozeného disku. 20% bylo zvoleno jako bezpečnostní rezerva pro očekávanou 5-10% ztrátu dat.</td>
</tr>

<tr>
<td align="center"><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů">  </td>
<td></td>
<td align="center"><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů">  </td>
<td></td>
</tr>

<tr>
<td><img src="../images/bad-cd.png" alt="Ikona: Poškozený disk (částečně nečitelný)"></td>
<td></td>
<td><img src="../images/ecc.png" alt="Ikona: Samostatný soubor s daty pro opravu chyb"></td>
<td> </td>
<td>Pokud později dojde k poškození disku, jsou data obnovena z jeho čitelné části a dat pro opravu chyb.</td>
</tr>

<tr>
<td align="right" class="w65x">80%<img src="../images/rdiag-arrow.png" alt="Ikona: Šipka šikmo doprava"></td>
<td></td>
<td align="left" class="w65x"><img src="../images/ldiag-arrow.png" alt="Ikona: Šipka šikmo doleva">20%</td>
<td></td>
<td>Pro úspěšnou obnovu musí na disku zbývat ještě minimálně 80% čitelných dat, zbývajících 20% je dopočítáno s pomocí dat pro opravu chyb.</td>
</tr>

<tr>
<td></td>
<td><img src="../images/good-image.png" alt="Ikona: Kompletní bitová kopie"></td>
<td></td>
<td></td>
<td>Kompletní obnovená data jsou k dispozici jako bitová kopie ISO na pevném disku (disk zůstává poškozený, protože fyzické poškození je nevratné).</td>
</tr>

<tr>
<td></td>
<td align="center"><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů"></td>
<td></td>
<td></td>
<td>Bitovou kopii vypalte pomocí vašeho vypalovacího programu na disk.</td>
</tr>

<tr>
<td></td>
<td align="center"><img src="../images/good-cd.png" alt="Ikona: Nepoškozený disk (bez chyb)"></td>
<td></td>
<td></td>
<td>Získáte tak nový nepoškozený disk.</td>
</tr>
</table><p>Jak jste si jistě všimli, obnova dat vyžaduje více kroků než běžné zálohování. Shrňme si tedy přednosti a nevýhody dvdisaster v porovnání s běžnými zálohami:<p><table>
<tr valign="top"><td>Výhody</td>
<td><ul>
<li>dvdisaster vyžaduje méně prostoru. Při použití dat pro opravu s 20% schopností opravy, vyžaduje ochrana 5 disků pouze jeden dodatečný disk pro opravná data.</li>
<li>Protože všechny disky stárnou a ke ztrátě dat dochází na podobných místech (obvykle na vnějším obvodu), nemusí kopie 1:1 pomoci. Obě kopie mohou být po pár letech poškozeny na stejných místech.</li>
</ul></td></tr>
<tr valign="top"><td>Podobnosti</td>
<td><ul><li>Jak záložní kopie, tak data pro opravu chyb musí být vytvořeny předtím než dojde k poškození původního disku. Nelze je vytvořit z poškozeného disku.</li></ul></td></tr>
<tr valign="top"><td>Nevýhody</td>
<td><ul><li>Pokud je překročena schopnost opravy dat pro opravu chyb (nebo dojde ke ztrátě disku), data nelze obnovit! Zejména vezměte na vědomí, že data pro opravu chyb s 20% schopností opravy a z 75% čitelný disk nelze použít k 95% obnově dat! V daném případě nelze obnovit více než oněch 75% čitelných dat!</li></ul></td></tr>
</table>Následující tři stránky obsahují podrobnější informace:<p><ul>
<li>Obecné vysvětlení pojmu <a href="howtos61.php">oprava chyb</a>.<p></li>
<li>Jane předvede <a href="howtos62.php">správného použití dvdisaster</a>. Vytvoří si předem data pro opravu chyb a bude tak schopna obnovit všechna svá data poté co dojde k poškození jejího disku.<p></li>
<li>Neměli byste však <a href="howtos63.php">následovat příklad</a> Joa. Nepoužívá data pro opravu chyb a zjistí tak, že jeho poškozené disky nelze obnovit ani po několika pokusech o přečtení. Následkem toho ztratí data z poškozeného disku.<p></li>
</ul>Tyto příběhy jsou pochopitelně zcela smyšlené a jakákoliv podobnost s existujícími osobami je čistě náhodná.<!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>