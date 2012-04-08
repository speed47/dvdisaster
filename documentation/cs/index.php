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

<!-- Insert actual page content here -->

<h3 class="top">Projekt dvdisaster:</h3>Optické disky (CD,DVD,BD) jsou schopné uchovat data jen po omezenou dobu (typicky několik let). Po uplynutí této doby dochází postupně ke ztrátě dat, kdy se objevují nečitelné oblasti postupující směrem od vnějšího okraje disku do středu.<p><b>Archivace s ochranou proti ztrátě dat</b><p>dvdisaster ukládá data na CD/DVD/BD (<a href="qa10.php#media">podporované disky</a>) takovým způsobem, že jsou obnovitelná i poté, co dojde k výskytu nečitelných oblastí. To umožňuje zachránit kompletní obsah disku a přenést ho na nový disk.<p>Ztrátě dat je zabráněno pomocí kódu pro opravu chyb. Data pro opravu chyb jsou buď přidána na chráněný disk, nebo uložena samostatně v souboru pro opravu chyb. Aby oprava nezávisela na systému souborů disku, pracuje dvdisaster na úrovni bitové kopie. Maximální kapacita opravy je volitelná.<p><b>Nejčastější omyly týkající se dvdisaster:</b><ul>
<li>dvdisaster nemůže učinit poškozené disky znovu čitelné. Obsah poškozeného disku <i>nemůže</i> být opraven bez dat pro opravu chyb.<p></li> 
<li><img src="images/exclude_from_search_terms.png" alt="" class="valignm"><br>Taková funkce je mimo rozsah návrhu a cílů vývoje dvdisaster.</li>
</ul>

<p><a href="index10.php">Příklady opravy chyb...</a> <?php
# Adds the footer line and closes the HTML properly.

end_page();
?>