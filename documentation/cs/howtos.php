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

<h3 class="top">Běžné použití</h3>dvdisaster je složitý nástroj, který by k popisu všech svých funkcí vyžadoval celou knihu. Protože právě postrádáme prostředky k vytvoření knihy (a vy také nemusíte mít dostatek času na čtení) pokusíme se použít trochu jiný přístup. Nejprve ukážeme, jak <a href="howtos60.php">jednotlivé funkce dvdisaster fungují dohromady</a>. Poté popíšeme několik běžných úloh a krok za krokem popíšeme jejich řešení. Postup podle těchto instrukcí bude ve většině případů vše co budete potřebovat. V závěru každého postupu jsou pak pro pokročilé uživatele uvedeny další možnosti konfigurace.<p><h3>Symboly používané v tomto dokumentu</h3>Práce s dvdisaster vyžaduje určité kombinace disků, jejich bitových kopií a dat pro opravu chyb. Prohlédněte a zapamatujte si následující symboly které vám řeknou co bude pro danou úlohu potřebovat:<p><b>Disk</b> (např. CD)<table cellspacing="10">
<tr>
<td align="center" class="w15p"><img src="../images/good-cd.png" alt="Ikona: Nepoškozený disk (bez chyb)"></td>
<td align="center" class="w15p"><img src="../images/bad-cd.png" alt="Ikona: Poškozený disk (částečně nečitelný)"></td>
<td class="w55p">Tyto symboly ukazují, zda je součástí dané úlohy práce s disky a zda musí být použitý disk naprosto bez chyb, nebo už může být poškozený.</td>
</tr>
<tr  valign="top">
<td>nepoškozený disk (<b>žádné</b> chyby čtení)</td>
<td>poškozený disk (<b>s</b> chybami čtení)</td>
<td></td>
</tr>
</table><p><b>Bitová kopie disku</b> (Bitová kopie disku ve formátu ISO uložená na pevném disku)<table cellspacing="10">
<tr>
<td align="center" class="w15p"><img src="../images/good-image.png" alt="Ikona: Kompletní bitová kopie"></td>
<td align="center" class="w15p"><img src="../images/bad-image.png" alt="Ikona: Nekompletní bitová kopie (z poškozeného disku)"></td>
<td class="w55p">Některé funkce nepracují přímo s diskem, ale s jeho bitovou kopií ve formátu ISO uloženou na pevném disku. V závislosti na stavu daného disku může být bitová kopie kompletní nebo nekompletní.</td>
</tr>
<tr valign="top">
<td>kompletní bitová kopie (vytvořena z nepoškozeného disku)</td>
<td>nekompletní bitová kopie (vytvořena z poškozeného disku)</td>
</tr>
</table><p><b>Data pro opravu chyb</b><table cellspacing="10">
<tr>
<td align="center" class="w15p"><img src="../images/good-cd-ecc.png" alt="Ikona: Disk obsahující data pro opravu chyb"></td>
<td align="center" class="w15p"><img src="../images/ecc.png" alt="Ikona: Samostatný soubor s daty pro opravu chyb"></td>
<td class="w55p">Oprava bitových kopií disků s využitím dat pro opravu chyb je hlavní funkcí dvdisaster. Tento symbol určuje, zda daná úloha vyžaduje data pro opravu chyb.</td>
</tr>
<tr  valign="top">
<td>Disk obsahující data pro opravu chyb</td>
<td>Samostatný soubor dat pro opravu chyb</td>
<td></td>
</tr>
</table><p><?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>