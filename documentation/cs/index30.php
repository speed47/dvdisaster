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

<h3 class="top">Výhody používání dvdisaster:</h3>

<ul>
<li><b>Chrání</b> před stárnutím a náhodným poškozením disku (v určitých mezích).<p></li>
<li><a href="howtos10.php">Kontroly čitelnosti</a> jsou <b>rychlejší</b> než kontroly kvality; v závislosti na mechanice dosahují až maximální rychlosti čtení.<p></li>
<li><b>Ekonomičnost:</b> disky musí být nahrazeny novými pouze pokud jsou opravdu poškozené.</li>
</ul>

<h3>Omezení použití dvdisaster:</h3>Potřebujete strategii zálohování a minimálně 15% dodatečného úložného prostoru.<ul>
<li>Data pro opravu chyb <b>musí být vytvořena předtím než disk selže</b>, nejlépe při jeho vytvoření.<p></li>
<li>Data pro opravu chyb vyžadují <b>dodatečný úložný prostor</b> na chráněném disku nebo na dodatečném samostatném disku. Při použití výchozího nastavení je potřebný dodatečný prostor přibližně 15% velikosti originálních dat (přibližně. 700MB pro plné 4.7GB DVD).<p></li>
<li>ochrana proti ztrátě dat není garantována.</li>
</ul>Pokud se o fungování dvdisaster chcete dozvědět více, prohlédněte si <a href="qa30.php">technické informace</a>.<!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>