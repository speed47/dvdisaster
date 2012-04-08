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
howto_headline("Zjištění informací o obrazech a datech pro opravu chyb", "Přehled", "images/compare-icon.png");
?>

<!-- Insert actual page content below -->

<table width="100%" cellspacing="5">
<tr valign="top">
<td class="w20p"><b>Úloha:</b></td>
<td>Zobrazení informací o typu a stavu bitové kopie a dat pro opravu chyb.</td>
</tr>
<tr><td><pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Požadavky:</b><p></td>
</tr>
<tr>
 <td class="w150x" align="right"><img src="../images/good-image.png" alt="Ikona: Kompletní bitová kopie" class="valignt"> <img src="../images/ecc.png" alt="Ikona: Samostatný soubor s daty pro opravu chyb"></td>
<td>Bitová kopie a volitelně i k ní příslušející data pro opravu chyb.</td>
<tr><td><pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Co udělat:</b><p></td>
</tr>

<tr>
<td></td>
<td>1. <a href="howtos51.php">Zobrazit informace</a><p>2. <a href="howtos51.php#examine">Interpretovat výsledky</a></td>
</tr>
</table><p><pre>


</pre><a href="howtos51.php">Zobrazení informací...</a> <!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.
end_page();
?>