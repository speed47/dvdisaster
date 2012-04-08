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

howto_headline("Vytvoření souborů pro opravu chyb", "Pokročilá nastavení", "images/create-icon.png");
?>

<?php begin_screen_shot("Záložka \"Mechanika\".","create-prefs-drive-adv.png"); ?><b>Po úspěšném načtení vysunout disk</b>. Tato funkce je užitečná pokud pracujete se sadou disků. Použijte ji společně s volbami v níže uvedeném snímku obrazovky.<p>dvdisaster se po vytvoření bitové kopie pokusí vysunout zdrojový disk. Vysunutí disku však může být zakázáno operačním systémem, takže u této volby není jisté, zda bude opravdu fungovat. Pokud se po vložení disku např. otevře okno s možností procházet obsah disku, nemusí být možné disk automaticky vysunout. <?php end_screen_shot(); ?><?php begin_screen_shot("Záložka \"Soubory\".","create-prefs-file-adv.png"); ?><b>Automatické vytváření a mazání souborů.</b> Pomocí těchto voleb můžete automatizovat proces vytváření souborů pro opravu chyb. První volba umožní dvdisaster vytvořit soubor pro opravu chyb ihned jakmile byl disk (kompletně) načten. Druhá volba pak po vytvoření souboru pro opravu chyb smaže bitovou kopii.<p><b>Poznámka:</b> Po vložení nového disku nezapomeňte nastavit nový název souboru pro opravu chyb. Jinak bude původní souboru pro opravu chyb přepsán.<?php end_screen_shot(); ?> <!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>