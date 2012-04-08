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

howto_headline("Rozšíření obrazu o data pro opravu chyb", "Pokročilá nastavení", "images/create-icon.png");
?>

<?php begin_screen_shot("Záložka \"Oprava chyb\".","create-prefs-ecc2-adv.png"); ?><b>Výběr velikosti bitové kopie</b>. dvdisaster obsahuje tabulku standardních velikostí CD, DVD a BD disků. Všechny disky by měly tyto požadavky na velikost splňovat. Někteří výrobci vyrábí disky s o něco větší velikostí. Pokud takový disk vlastníte, vložte ho do mechaniky a klikněte na tlačítko &quot;podle disku&quot; (označeno zeleně) vedle příslušného typu disku. dvdisaster zjistí velikost vloženého disku a příslušně tabulku aktualizuje.<p><b>Poznámka:</b> Velikost disku může být zjištěna pouze v mechanice, která je schopná daný typ disku vypálit.<pre> </pre><b>Volitelná velikost bitové kopie.</b> Můžete nastavit konkrétní velikost bitové kopie, která nemá být po rozšíření bitové kopie o data pro opravu chyb překročena. Pokud tak chcete učinit, aktivujte volbu &quot;Použít nejvíce ... sektorů&quot; (označeno žlutě) a zadejte maximální velikost bitové kopie v sektorech (1 sektor = 2KB). <?php end_screen_shot(); ?> <!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>