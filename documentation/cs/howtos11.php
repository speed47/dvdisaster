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

howto_headline("Kontrola poškození disků", "Základní nastavení", "images/scan-icon.png");
?>

<!-- Insert actual page content below -->

<?php begin_screen_shot("Otevření dialogu nastavení.","global-prefs-invoke.png"); ?>
<table><tr><td class="valignt"><img src="../images/prefs-icon.png" alt="Ovládací prvky dvdisaster: Nastavení (tlačítko)" class="valignb"></td>
<td>Následující záložky naleznete v dialogu nastavení. Dialog nastavení otevřete pomocí zeleně označeného tlačítka z následujícího snímku obrazovky (obrázek zvětšíte kliknutím na něj). Symbol na tlačítku se může lišit v závislosti na vámi používaném vzhledu GTK rozhraní.</td>
</tr></table>
<?php end_screen_shot(); ?><a name="image"></a> <?php begin_screen_shot("záložka \"Bitová kopie\".","scan-prefs-image.png"); ?> <b>Záložka &quot;Bitová kopie&quot;.</b> Je důležité vybrat správnou metodu určení velikosti bitové kopie. Volba &quot;ISO/UDF&quot; (označená zeleně) většinou funguje ve všech případech. Volbu &quot;ECC/RS02&quot; (označenou červeně) vyberte pouze v případě, že jste si jisti, že jsou na disku uložena RS02 data pro opravu chyb. Využití dat pro opravu chyb (ECC) zlepší výsledky kontroly, ale hledání neexistujících ECC dat kontrolu neúměrně prodlouží.<p>Ostatní volby nastavte tak, jak je uvedeno v následujícím snímku obrazovky.<p><?php end_screen_shot(); ?><pre> </pre>

<?php begin_screen_shot("Záložka \"Mechanika\".","scan-prefs-drive.png"); ?><b>Záložka &quot;Mechanika&quot;.</b> Při čtení dat během roztáčení mechaniky může dojít k výskytu falešných hlášení chyb. V zeleně označeném poli přizpůsobte čas na roztočení požadavkům vaší mechaniky (typicky 5-10 sekund), tak aby dvdisaster začal se čtením ve správný čas.<p>Ostatní nastavení ponechte tak jak jsou, můžete je <a href="howtos14.php">optimalizovat</a> později.<p><?php end_screen_shot(); ?><pre> </pre><a name="read_attempts"></a> <?php begin_screen_shot("Záložka \"Pokusy o přečtení\" tab.","scan-prefs-read-attempts.png"); ?> <b>Záložka &quot;Pokusy o přečtení&quot;.</b> Nastavte volby pokusů o přečtení jak je uvedeno zde. Použití vyšších hodnot způsobí vyšší zátěž mechaniky, ale přesnost kontroly nezlepší. Volba &quot;Použít přímé čtení a analýzu sektorů&quot; (první zeleně označená volba) používá analýzu C2 a potenciálně více dat obdržených od mechaniky v tomto režimu pro lepší vyhodnocení kvality CD disků. Tato volba nemá žádný vliv na kontrolu DVD a BD disků, ale pokud nezpůsobuje s vaší mechanikou problémy při čtení CD, je možné ji nechat trvale aktivní. Po výskytu chyby čtení nemá být přeskočeno méně než 16 sektorů (druhá zeleně označená volba); při čtení hodně poškozených disků může být tato volba <a href="howtos14.php">optimalizována použitím vyšších hodnot</a>.<br>Při kontrole není doporučeno více pokusů o čtení; nastavte hodnotu opakování u všech tří oranžově označených voleb na 1. Během kontroly by mělo být vypnuto i ukládání nezpracovaných sektorů.<p><?php end_screen_shot(); ?><pre> </pre>

<?php begin_screen_shot("Záložka \"Ostatní\".","general-prefs-misc.png"); ?><b>Záložka &quot;Ostatní&quot;.</b> Tato záložka obsahuje prozatím jen funkce pro vytváření souborů se záznamem. Tyto volby jsou užitečné při zasílání <a href="feedback.php">hlášení o chybě</a> ale při běžném používání by měly být vypnuté. <?php end_screen_shot(); ?><pre> </pre><b>Nepoužité záložky</b><p>Záložky &quot;Oprava chyb&quot; a &quot;Soubory&quot; nemají žádný vliv na funkci kontroly disků. Záložka &quot;Vzhled&quot; umožňuje nastavit barvy používané k zobrazení informací podle vašeho vkusu, ale nemá žádný další vliv na průběh kontroly.<pre> </pre><a href="howtos12.php">Provedení kontroly...</a> <!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>