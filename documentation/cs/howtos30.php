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
howto_headline("Rozšíření obrazu o data pro opravu chyb", "Přehled", "images/create-icon.png");
?>

<!-- Insert actual page content below -->

<h3 class="top">Umístění dat pro opravu chyb přímo na chráněný disk</h3>

<table width="100%" cellspacing="5">
<tr valign="top">
<td class="w20p"><b>Úloha:</b></td>
<td>Uložení dat pro opravu chyb společně s chráněnými uživatelskými daty na jeden disk.</td>
</tr>
<tr><td><pre> </pre> </td></tr>

<tr valign="top">
<td></td>
<td>Poznámka: Tato stránka popisuje rozšíření bitové kopie ve formátu ISO o data pro opravu chyb před jejím vypálením na disk. Existuje také metoda pro vytvoření samostatného souboru s daty pro opravu chyb. <a href="howtos21.php">Chcete pomoci s rozhodováním mezi těmito dvěma metodami?</a></td>
<tr><td><pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Požadavky:</b><p></td>
</tr>

<tr>
<td><img src="../images/good-image.png" alt="Ikona: Kompletní bitová kopie"></td>
<td><ul>
<li>program pro vytváření (&quot;vypalování&quot;) schopný vytvářet bitové kopie ve formátu ISO</li>
<li>disk který má být rozšířen o data pro opravu chyb ještě nebyl vypálen <a href="#footnote"><sup>*)</sup></a></li>
<li>na vytvářeném disku je nejméně 20% volného prostoru</li>
</ul>
</tr>
<tr><td><pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Co udělat:</b><p></td>
</tr>

<tr>
<td></td>
<td>1. <a href="howtos32.php">Provést základní nastavení</a><p>2a. <a href="howtos33.php#a">Vytvořit bitovou kopii ve formátu ISO,</a><br>2b. <a href="howtos33.php#b">rozšířit ji o data pro opravu chyb,</a><br>2c. <a href="howtos33.php#c">a zapsat ji na disk.</a></td>
</tr>
</table><p><a href="howtos32.php">Základní nastavení...</a><pre>


</pre>

<!-- do not change below -->

<?php
footnote("*","footnote","Vypálený disk nemůže být rozšířen o data pro opravu chyb.");

# end_page() adds the footer line and closes the HTML properly.
end_page();
?>
