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
require("../include/footnote.php");
begin_page();
howto_headline("Vytvoření samostatného souboru s daty pro opravu chyb", "Přehled", "images/create-icon.png");?>

<!-- Insert actual page content below -->

<table width="100%" cellspacing="5">
<tr valign="top">
<td class="w20p"><b>Úloha:</b></td>
<td>Vytvoření souboru s daty pro opravu chyb pro CD/DVD/BD disky.</td>
</tr>
<tr><td><pre> </pre> </td></tr>

<tr valign="top">
<td></td>
<td>Poznámka: Tato stránka popisuje vytváření a uchovávání samostatných souborů s daty pro opravu chyb. Existuje také metoda umožňující přímé uložení dat pro opravu chyb na chráněný disk. <a href="howtos21.php">Chcete pomoci s rozhodováním mezi těmito dvěma metodami?</a></td>
<tr><td><pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Požadavky:</b><p></td>
</tr>

<tr>
<td><img src="../images/good-cd.png" alt="Ikona: Nepoškozený disk (bez chyb)"></td>
<td>Nepoškozený<a href="#footnote"><sup>*)</sup></a> disk,</td>
</tr>

<tr><td></td><td>nebo</td></tr>


<tr>
<td><img src="../images/good-image.png" alt="Ikona: Kompletní bitová kopie"></td>
<td>existující a kompletní<a href="#footnote"><sup>*)</sup></a> bitová kopie disku ve formátu ISO (např. bitová kopie použitá k vytvoření disku).</td>
</tr>
<tr><td><pre> </pre> </td></tr>


<tr>
<td colspan="2"><b>Co udělat:</b><p></td>
</tr>

<tr>
<td></td>
<td>1. <a href="howtos22.php">Provést základní nastavení</a><br>2. <a href="howtos23.php">Vytvořit soubor pro opravu chyb</a><br>3. <a href="howtos24.php">Zazálohovat soubor pro opravu chyb</a></td>
</tr>
</table><p><a href="howtos22.php">Základní nastavení...</a><pre>


</pre>

<?php
footnote("*","footnote","Data pro opravu chyb musí být vytvořena předtím než dojde k poškození dat: není možné vytvořit data pro opravu chyb pro poškozený disk.");
?>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
