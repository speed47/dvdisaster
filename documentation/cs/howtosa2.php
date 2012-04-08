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

<h3 class="top">Výběr souboru bitové kopie</h3>Soubor bitové kopie obsahuje data všech sektorů disku, včetně informací o čitelnosti sektoru. dvdisaster pracuje s bitovými kopiemi, protože jsou uloženy na pevném disku, díky čemuž je provádění některých vzorců náhodného přístupu mnohem rychlejší. Použití podobného typu náhodného přístupu u CD/DVD/BD mechaniky by vedlo k výraznému zpomalení a k jejich rychlejšímu opotřebení (bitové kopie jsou vytvářeny pomocí sekvenčního čtení, které na rozdíl od náhodného čtení mechaniky zvládají dobře). Výchozí přípona pro soubory bitových kopií je ISO.<p><?php begin_screen_shot("Výběr souboru bitové kopie","dialog-iso-full.png"); ?>Existují dva způsoby výběru souboru bitové kopie:<ul>
<li>pomocí <a href="#filechooser">dialogu pro výběr souboru</a> (zeleně označené tlačítko), nebo</li>
<li>přímé zadání umístění souboru (modře označená položka).</li>
</ul><p>Přímý vstup je užitečný pokud zpracováváte několik souborů v jednom adresáři. V tomto případě stačí změnit pouze název souboru ve vstupním poli.<p><?php end_screen_shot(); ?> <?php require("howtos_winfile.php"); ?> <!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>