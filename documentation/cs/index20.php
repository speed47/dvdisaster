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

<h3 class="top">dvdisaster jako doplněk kontrol kvality</h3><a href="qa.php#pipo">Kontrola kvality</a>, např. zjištění C2 chyb nebo PI/PO chyb, je užitečná ke zjištění kvality vypálení disku.<p>Ale kontrola kvality <b>není</b> spolehlivým způsobem <b>odhadnutí životnosti</b> optického disku. <br>Předpokládejme, že hledáme nejlepší okamžik pro zkopírování opotřebovaného disku na nový:<ul>
<li>Příliš brzy: kopírování disku z důvodu špatných výsledků kontroly kvality není ekonomické. Takové disky někdy zůstanou čitelné mnohem déle než se dalo předpokládat.<p></li>
<li>Příliš pozdě: Když už kontrola kvality odhalí nečitelné sektory, došlo již ke ztrátě některých dat.<p></li>
<li>Těsně před selháním disku: Ideální stav, ale jak ho zjistit?</ul>Můžeme to ale provést pomocí dvdisaster:<ul>
<li>Pro disk <a href="howtos20.php">vytvoříme data por opravu chyb</a>.<p></li>
<li>Pravidelně <a href="howtos10.php">disk kontrolujeme</a>. Disk používáme dokud se neobjeví první chyby.<p></li>
<li>Chyby <a href="howtos40.php">opravíme</a> <a href="howtos40.php">pomocí dat pro opravu chyb</a>. Opravenou bitovou kopii zapíšeme na nový disk.</li>
</ul>

<p><a href="index30.php">Přednosti a nevýhody dvdisaster...</a> <!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>