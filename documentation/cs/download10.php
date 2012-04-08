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

<h3 class="top">Systémové požadavky</h3>

<ul>
 <li>Procesory: x86, PowerPC nebo Sparc;</li>
 <li>aktuální CD/DVD/BD mechanika s ATAPI nebo SCSI rozhraním;</li>
 <li>dostatek volného prostoru pro vytvoření bitových kopií zpracovávaných disků.</ul>
<p><h3>Podporované operační systémy</h3>V následující tabulce je uveden přehled podporovaných operačních systémů. Uvedené verze byly použity při vývoji a testování aktuální verze dvdisaster. Typicky by měly být podporovány i o něco starší nebo novější verze.<p>Projekt dvdisaster doporučuje GNU/Linux.<p><table border="1">
<tr>
<td>Operační systém</td>
<td>Verze</td>
<td>Podpora 32bitů</td>
<td>Podpora 64bitů</td>
</tr>
<tr>
<td>GNU/Linux</td>
<td>Debian Squeeze (6.0.4)<br>kernel 2.6.32</td>
<td align="center">ano</td>
<td align="center">ano</td>
</tr>
<tr>
<td>FreeBSD<sup>1)</sup></td>
<td>9.0</td>
<td align="center">ano</td>
<td align="center">ano</td>
</tr>
<tr>
<td>NetBSD</td>
<td>5.1.2</td>
<td align="center">ano</td>
<td align="center">ano</td>
</tr>
<tr>
<td>Mac OS X</td>
<td>10.6 (Snow Leopard)</td>
<td align="center">ano</td>
<td align="center">ne<sup>2)</sup></td>
</tr>
<tr>
<td>Windows<sup>4)</sup></td>
<td>Windows 2000 SP4<sup>3)</sup></td>
<td align="center">ano</td>
<td align="center">ne<sup>2)</sup></td>
</tr>
</table><p><sup>1)</sup>FreeBSD: použití ATAPI vyžaduje načtení jaderného modulu <i>atapicam</i> -- podrobnosti v souboru INSTALL<br> <sup>2)</sup>Podpora pro 64bitů není plánována.<br> <sup>3)</sup>Byla potvrzena funkčnost v novějších verzích včetně Windows 7. Windows 2000 SP3 a starší nejsou podporovány.<br> <sup>4)</sup>Podpora vícejádrových procesorů není stabilní. V některých verzích se při použití více jader neprojeví žádné zvýšení výkonu dvdisaster.<!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>