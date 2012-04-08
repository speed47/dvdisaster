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

<h3 class="top">Příklady opravy chyb</h3>

<?php begin_screen_shot("Načtení poškozeného disku.", "recover-linear.png"); ?><b>Oprava starého disku.</b> U zde zpracovávaného disku došlo k odbarvení a ztrátě čitelnosti oblastí na jeho vnějším okraji. Kontrola povrchu zjistila přibližně 23.000 nečitelných sektorů z 342.000 sektorů celkem; výsledkem je tedy přibližně 7,2% poškozených sektorů. <?php end_screen_shot(); ?> <?php begin_screen_shot("Oprava poškozené bitové kopie.", "fix-image.png"); ?><b>Oprava poškozené bitové kopie.</b> Vytvořená bitová kopie je stále nekompletní, protože se nepodařilo načíst přibližně 23.000 sektorů. Tyto sektory jsou nyní opraveny pomocí dat pro opravu chyb vytvořených prostřednictvím dvdisaster. Během opravy je dosaženo maximálně 20 chyb na jeden opravný blok. Výsledkem je maximální zatížení opravy chyb z 63%, což znamená, že tento typ poškození lze bez problémů opravit pomocí dat pro opravu chyb vytvořených s pomocí výchozího nastavení. <?php end_screen_shot(); ?><b>Oprava vyžaduje data pro opravu chyb:</b> Výše popsaný proces opravy používá data pro opravu chyb (ECC). Tyto data si představte jako speciální formu zálohy (vyžaduje ale méně prostoru než běžná záloha). Stejně jako běžná záloha, tak i ECC data musí být vytvořena <i>před</i> poškozením disku.<p>Takže pokud máte poškozený disk pro který jste nevytvořili ECC data - litujeme, ale vaše data jsou pravděpodobně ztracena.<p><a href="index20.php">Proč kontroly kvality nestačí...</a> <!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>