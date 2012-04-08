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
require("../include/screenshot.php");
begin_page();

?>

<!-- Insert actual page content below -->

<h3 class="top">Výběr mechaniky</h3>

<?php begin_screen_shot("Výběr mechaniky","dialog-drive-full.png"); ?>Nabídka pro výběr mechanik je umístěna v levém horním rohu panelu nástrojů (označeno zeleně). Pro zobrazení rozevírací nabídky mechanik klikněte do pole nalevo od symbolu disku. Poté vyberte mechaniku obsahující disk který chcete zpracovat pomocí dvdisaster.<p>Pro zjednodušení identifikace mechanik jsou u jednotlivých položek uvedeny následující informace:<ul>
<li>Identifikace mechaniky sestávající běžně z názvu výrobce a typu mechaniky. Tyto hodnoty byly u mechaniky nastaveny výrobcem. Protože jsou tyto informace zobrazeny bez jakékoliv úpravy, uvidíte to co výrobce považoval za vhodné. Někdy tato identifikace není příliš smysluplná.<p></li>
<li>Popis pod kterým je mechanika spravována operačním systémem (např. /dev/hda v GNU/Linux nebo F: ve Windows)</li>
</ul>
<?php end_screen_shot(); ?>


<p><b>Příklady:</b><table width="100%">
<tr>
<td class="w50p" align="center"><img src="images/select-drive-linux.png" alt="Ovládací prvky dvdisaster: Výběr mechaniky v Linuxu"><br>Otevřená nabídka pod GNU/Linux</td>
<td class="w50p" align="center"><img src="images/select-drive-win.png" alt="Ovládací prvky dvdisaster: Výběr mechaniky ve Windows"><br>Otevřená nabídka ve Windows</td>
</tr>
</table><p><!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>