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

howto_headline("Kontrola poškození disků", "Pokročilá nastavení", "images/scan-icon.png");
?>

<!-- Insert actual page content below -->

<?php begin_screen_shot("Záložka \"Mechanika\".","scan-prefs-drive-adv.png"); ?><b>Ignorování neopravitelných chyb.</b>Pokud mechanika nahlásí neopravitelnou chybu, jako jsou např. mechanické problémy, dvdisaster kontrolu přeruší. Důvodem je zabránění poškození mechaniky. Některé mechaniky však podobné chyby hlásí pokud jsou zmateny při čtení poškozeného disku. Pokud vlastníte některou z těchto mechanik, můžete pomocí této volby vynutit pokračování kontroly.<p><b>Po úspěšném načtení vysunout disk.</b> Pokud tuto volbu aktivujete, dvdisaster se disk po úspěšném přečtení pokusí vysunout. Vysunutí disku však může být zakázáno operačním systémem, takže u této volby není jisté, zda bude opravdu fungovat. Pokud se po vložení disku např. otevře okno s možností procházet obsah disku, nemusí být možné disk automaticky vysunout.<p><?php end_screen_shot(); ?><pre> </pre>

<?php begin_screen_shot("Záložka \"Pokusy o přečtení\".","scan-prefs-read-attempts-adv.png"); ?><b>Přeskakování sektorů po chybě čtení.</b> Pokusy o přečtení poškozených sektorů trvají velice dlouho. Protože je velice pravděpodobné, že po chybě čtení narazíte na další poškozený sektor, je přeskočení několika sektorů po chybě čtení dobrým způsobem, jak ušetřit čas a snížit zátěž mechaniky. Pokud požadujete pouze hrubý přehled o poškození, může pomoci nastavit tuto hodnotu na 1024. Nezapomeňte ale, že všechny přeskočené sektory jsou považovány za nečitelné, takže počet nahlášených chyb bude vyšší a méně přesný.<p><?php end_screen_shot(); ?> <!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>