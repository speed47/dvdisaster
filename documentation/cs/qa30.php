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

<h3 class="top">Technické informace</h3>Informace této sekce nejsou nezbytné k používání programu dvdisaster. Je ale užitečné vědět jak dvdisaster funguje, může vám to totiž pomoci program maximálně využít.<ol>
<li><a href="qa31.php">Vlastnosti opravy chyb Reed-Solomon</a><p></li>
<li><a href="qa32.php">Oprava chyb na úrovni bitové kopie</a><p></li>
<li><a href="qa33.php">Metody RS01, RS02 a RS03</a><p></li>
<li><a href="qa34.php">Podrobnosti o lineární strategii čtení</a><p></li>   
<li><a href="qa35.php">Podrobnosti o adaptivní strategii čtení</a><p></li>   
<li><a href="qa36.php">Poznámky k chybám čtení</a><p></li>   
<li><a href="qa37.php">Tipy pro uchovávání souborů pro opravu chyb</a><p></li>
</ol>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
