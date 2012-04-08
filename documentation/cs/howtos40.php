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
?>

<!-- Insert actual page content below -->

<h3 class="top">Oprava bitových kopií disků</h3>

<table width="100%" cellspacing="5">
<tr valign="top">
<td class="w20p"><b>Úloha:</b></td>
<td>Oprava obsahu poškozeného disku.</td>
</tr>
<tr><td><pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Požadavky:</b><p></td>
</tr>
<tr>
 <td class="w150x" align="right"><img src="../images/bad-cd-ecc.png" alt="Ikona: Poškozený disk s daty pro opravu chyb" class="valignt"></td>
<td>Poškozený disk obsahující <a href="howtos30.php">data pro opravu chyb</a>,</td>
</tr>
<tr><td></td><td>nebo</td></tr>
<tr>
 <td class="w150x" align="right"><img src="../images/bad-cd.png" alt="Ikona: Poškozený disk (částečně nečitelný)"> <img src="../images/ecc.png" alt="Ikona: Samostatný soubor s daty pro opravu chyb"></td>
<td>poškozený disk s odpovídajícím <a href="howtos20.php">souborem pro opravu chyb</a><a href="#footnote"><sup>*)</sup></a>.</td>
</tr>
<tr><td><pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Co udělat:</b><p></td>
</tr>

<tr>
<td></td>
<td>1. <a href="howtos41.php">Provést základní nastavení čtení,</a><br>2a. <a href="howtos42.php#a">vytvořit bitovou kopii ISO poškozeného disku,</a><br>2b. <a href="howtos42.php#b">opravit bitovou kopii a zapsat ji na nový disk.</a></td>
</tr>
</table><p><a href="howtos42.php">Vytvoření a oprava bitové kopie...</a><pre>


</pre>

<!-- do not change below -->
<?php
footnote("*","footnote",
"Soubor pro opravu chyb musel být vytvořen v době kdy disk ještě nebyl poškozen: není možné vytvořit data pro opravu chyb pro poškozený disk.");
# end_page() adds the footer line and closes the HTML properly.
end_page();
?>
