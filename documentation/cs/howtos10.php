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

howto_headline("Kontrola poškození disků", "Přehled", "images/scan-icon.png");
?>

<!-- Insert actual page content below -->

<table width="100%" cellspacing="5">
<tr>
 <td><b>Úloha:</b></td>
 <td>Vyhledání nečitelných sektorů na disku.</td>
</tr>
<tr><td><pre> </pre> </td></tr>

<tr>
 <td colspan="2"><b>Požadavky:</b></td>
</tr>
<tr>
 <td class="w150x"><img src="../images/good-cd.png" alt="Ikona: Nepoškozený disk (bez chyb)" class="valignt">   <img src="../images/bad-cd.png" alt="Ikona: Poškozený disk (částečně nečitelný)" class="valignt"></td>
<td>Disk v jakémkoliv stavu (nepoškozený nebo poškozený).</td>
</tr>

<tr>
 <td><img src="../images/ecc.png" alt="Ikona: Samostatný soubor s daty pro opravu chyb"></td>
 <td>Pokud jsou k dispozici data pro opravu chyb, jsou provedeny dodatečné testy. Kontrola ale bude funkční i bez dat pro opravu chyb.</td>
</tr>

<tr><td><pre> </pre> </td></tr>

<tr valign="top">
 <td><b>Co udělat:</b></td>
 <td>1. <a href="howtos11.php">Provést základní nastavení</a><br>2. <a href="howtos12.php">Spustit kontrolu disku</a><br>3. <a href="howtos13.php">Interpretovat výsledky</a><br></td>
</tr>

<tr><td><pre> </pre> </td></tr>

<tr valign="top">
 <td><b>Související funkce:</b></td>
 <td><a href="howtos42.php#a">Čtení poškozených disků</a> a<br> <a href="howtos40.php">Oprava bitových kopií</a>.</td>
 </tr>
</table><p><pre> </pre><a href="howtos11.php">Základní nastavení...</a> <!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>