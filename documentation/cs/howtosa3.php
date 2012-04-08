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

<h3 class="top">Výběr souboru pro opravu chyb</h3>Soubor pro opravu chyb obsahuje informace pro opravu nečitelných sektorů poškozeného disku. Může také sloužit ke kontrole poškození nebo pozměnění disku. Výchozí přípona souboru je ECC.<p><?php begin_screen_shot("Výběr souboru pro opravu chyb","dialog-ecc-full.png"); ?>Existují dva způsoby výběru souboru pro opravu chyb:<ul>
<li>pomocí <a href="#filechooser">dialogu pro výběr souboru</a> (zeleně označené tlačítko), nebo</li>
<li>přímé zadání umístění souboru pro opravu chyb (modře označená položka).</li>
</ul><p>Přímý vstup je užitečný pokud zpracováváte několik souborů pro opravu chyb v jednom adresáři. V tomto případě stačí změnit pouze název souboru ve vstupním poli.<p><?php end_screen_shot(); ?> <?php require("howtos_winfile.php"); ?> <!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>