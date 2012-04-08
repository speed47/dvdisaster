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

howto_headline("Rozšíření obrazu o data pro opravu chyb", "Základní nastavení", "images/create-icon.png");
?>

<!-- Insert actual page content below -->

<?php begin_screen_shot("Otevření dialogu nastavení.","global-prefs-invoke.png"); ?>
<table><tr><td class="valignt"><img src="../images/prefs-icon.png" alt="Ovládací prvky dvdisaster: Nastavení (tlačítko)" class="valignb"></td>
<td>Následující záložky naleznete v dialogu nastavení. Dialog nastavení otevřete pomocí zeleně označeného tlačítka z následujícího snímku obrazovky (obrázek zvětšíte kliknutím na něj). Symbol na tlačítku se může lišit v závislosti na vámi používaném vzhledu GTK rozhraní.</td>
</tr></table>
<?php end_screen_shot(); ?>

<pre> </pre>

<?php begin_screen_shot("Záložka \"Oprava chyb\".","create-prefs-ecc2.png"); ?><b>Záložka &quot;Oprava chyb&quot;.</b> Jako metodu uložení vyberte &quot;Rozšířená bitová kopie (RS02)&quot; (zelená nabídka). Pokud používáte disky standardní velikosti vyberte &quot;Použít nejmenší možnou velikost z následující tabulky&quot;. dvdisaster pak pro uložení bitové kopie vybere nejmenší použitelnou velikost disku. Bitová kopie pak bude příslušným způsobem rozšířena a zbývající volný prostor bude použit k uložení dat pro opravu chyb. <?php end_screen_shot(); ?><pre> </pre><b>Nepoužité záložky</b><p>Záložka &quot;Ostatní&quot; prozatím obsahuje pouze funkce pro vytváření souborů se záznamem. Tyto volby jsou užitečné při zasílání <a href="feedback.php">hlášení o chybě</a>, ale při běžném používání by měly být vypnuté. Záložka &quot;Vzhled&quot; umožňuje nastavit barvy používané k zobrazení informací podle vašeho vkusu, ale nemá žádný vliv na tvorbu dat pro opravu chyb.<pre> </pre><a href="howtos33.php">Rozšíření bitové kopie o data pro opravu chyb...</a> <!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>