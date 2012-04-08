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

howto_headline("Oprava bitových kopií disků", "Základní nastavení", "images/fix-icon.png");
?>

<!-- Insert actual page content below -->

<?php begin_screen_shot("Otevření dialogu nastavení.","global-prefs-invoke.png"); ?>
<table><tr><td class="valignt"><img src="../images/prefs-icon.png" alt="Ovládací prvky dvdisaster: Nastavení (tlačítko)" class="valignb"></td>
<td>Následující záložky naleznete v dialogu nastavení. Dialog nastavení otevřete pomocí zeleně označeného tlačítka z následujícího snímku obrazovky (obrázek zvětšíte kliknutím na něj). Symbol na tlačítku se může lišit v závislosti na vámi používaném vzhledu GTK rozhraní.</td>
</tr></table>
<?php end_screen_shot(); ?>Volby uvedené v této sekci nastaví dvdisaster pro čtení poškozených disků. Neexistují žádná zvláštní nastavení pro opravu bitové kopie pomocí dat pro opravu chyb.<pre> </pre>

<?php begin_screen_shot("Záložka \"Bitová kopie\".","fix-prefs-image.png"); ?><b>Záložka &quot;Bitová kopie&quot;.</b> Nejdříve vyberte typ dat pro opravu chyb. Pokud máte soubor pro opravu chyb zvolte &quot;ISO/UDF&quot; (označeno zeleně). Pokud jde o disk který byl rozšířen o data pro opravu chyb, vyberte &quot;ECC/RS02&quot; (označeno modře).<p>Adaptivní režim čtení používá údaje dat pro opravu chyb pro maximalizaci efektivnosti čtení. Aktivujte ho pomocí žlutě označené volby.<p>Ostatní nastavení ponechte tak, jak jsou uvedena na snímku obrazovky.<p><?php end_screen_shot(); ?> <?php begin_screen_shot("Záložka \"Mechanika\".","fix-prefs-drive.png"); ?> <b>Záložka &quot;Mechanika&quot;.</b> Prozatím ponechte na snímku uvedené výchozí nastavení;. Některé mechaniky mohou lépe fungovat při použití režimu přímého čtení &quot;21h&quot;. Více informací je uvedeno v <a href="howtos43.php#21h">pokročilých nastaveních</a>.<p><?php end_screen_shot(); ?> <a name="reading_attempts"></a> <?php begin_screen_shot("Záložka \"Pokusy o přečtení\".","fix-prefs-read-attempts.png"); ?> <b>Záložka &quot;Pokusy o přečtení&quot;.</b> Síla adaptivní strategie čtení je v hledání čitelných sektorů za současného předcházení zdlouhavých pokusů o čtení poškozených sektorů. Vyberte proto &quot;přímé&quot; čtení (označeno zeleně) protože extrakci nijak nezpomalí, ale snižte počet pokusů o přečtení na minimum (označeno žlutě). Jako kritérium ukončení prvního pokusu o přečtení použijte 128 nečitelných sektorů (označeno modře). Zatím nezapínejte ukládání nezpracovaných sektorů. Pokud s tímto výchozím nastavením nezískáte dostatek dat pro opravu, můžete je <a href="howtos43.php">optimalizovat</a> později.<p><?php end_screen_shot(); ?><pre> </pre><b>Nepoužité záložky</b><p>Záložka &quot;Oprava chyb&quot; nemá žádný vliv na průběh čtení. Záložka &quot;Ostatní&quot; prozatím obsahuje pouze funkce pro vytváření souborů se záznamem. Tyto volby jsou užitečné při zasílání <a href="feedback.php">hlášení o chybě</a>, ale při běžném používání by měly být vypnuté. Záložka &quot;Vzhled&quot; umožňuje nastavit barvy používané k zobrazení informací podle vašeho vkusu, ale nemá žádný další vliv na průběh čtení.<pre> </pre><a href="howtos42.php">Čtení disku a oprava obsahu...</a> <!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>