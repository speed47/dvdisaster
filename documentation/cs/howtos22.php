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

howto_headline("Vytvoření souborů pro opravu chyb", "Základní nastavení", "images/create-icon.png");
?>

<!-- Insert actual page content below -->

<?php begin_screen_shot("Otevření dialogu nastavení.","global-prefs-invoke.png"); ?>
<table><tr><td class="valignt"><img src="../images/prefs-icon.png" alt="Ovládací prvky dvdisaster: Nastavení (tlačítko)" class="valignb"></td>
<td>Následující záložky naleznete v dialogu nastavení. Dialog nastavení otevřete pomocí zeleně označeného tlačítka z následujícího snímku obrazovky (obrázek zvětšíte kliknutím na něj). Symbol na tlačítku se může lišit v závislosti na vámi používaném vzhledu GTK rozhraní.</td>
</tr></table>
<?php end_screen_shot(); ?>

<hr><a name="načtení"><b>Nastavení pro vytvoření bitové kopie disku</b></a><p><table width="100%" cellspacing="5">
<tr>
<td><img src="../images/good-image.png" alt="Ikona: Kompletní bitová kopie"></td>
<td>Pokud již máte k dispozici bitovou kopii ve formátu ISO, můžete následující dvě záložky přeskočit a pokračovat od <a href="#ecc">nastavení opravy chyb</a>. Ujistěte se ale, že jde opravdu o bitovou kopii ve formátu ISO; ostatní formáty bitových kopií, jako je například NRG, nelze použít k vytvoření použitelných dat pro opravu chyb.</td>
</tr>
</table><p><?php begin_screen_shot("Záložka \"Bitová kopie\".","create-prefs-image.png"); ?> <b>Záložka &quot;Bitová kopie&quot;.</b> Ujistěte se, že pro zjištění velikosti bitové kopie je nastavena volba &quot;ISO/UDF&quot; a že je vybrána lineární strategie čtení. Požadovaná nastavení jsou označena zeleně. Ostatní volby ponechte na jejich výchozích hodnotách.<p><?php end_screen_shot(); ?><pre> </pre>

<?php begin_screen_shot("Záložka \"Mechanika\".","create-prefs-drive.png"); ?><b>Záložka &quot;Mechanika&quot;.</b> Při čtení dat během roztáčení mechaniky může dojít k výskytu falešných hlášení chyb. V zeleně označeném poli přizpůsobte čas na roztočení požadavkům vaší mechaniky (typicky 5-10 sekund), tak aby dvdisaster začal se čtením ve správný čas.<p>Ostatní volby ponechte na zobrazených hodnotách.<p><?php end_screen_shot(); ?> <?php begin_screen_shot("\"Pokusy o přečtení\".","create-prefs-read-attempts.png"); ?> <b>&quot;Pokusy o přečtení&quot;.</b> Volba &quot;Použít přímé čtení a analýzu sektorů&quot; (označená zeleně) používá dodatečné informace poskytované mechanikou k ověření integrity dat. To je doporučeno, protože chceme vytvořit data pro opravu chyb pro bezchybně vytvořenou bitovou kopii. Na druhou stranu, protože data pro opravu chyb mohou být vytvořena jen pro bezchybně čitelné disky, nepotřebujeme mít povoleny opakované pokusy o čtení a ukládání nezpracovaných sektorů.<?php end_screen_shot(); ?><hr><a name="ecc"><b>Nastavení opravy chyb</b></a><p><?php begin_screen_shot("\"Oprava chyb\" tab.","create-prefs-ecc.png"); ?> <b>Záložka &quot;Oprava chyb&quot;.</b> Nejprve z nabídky &quot;Způsob uložení&quot; zvolte &quot;Soubor pro opravu chyb (RS01)&quot; (označeno zeleně). Nastavením redundance určíte maximální schopnost opravy chyb: soubor pro opravu chyb s x% redundancí může v optimálním případě opravit maximálně x% chyb čtení. Protože optimálních podmínek se málokdy dosáhne, měli by jste při nastavování redundance volbou některé z nabízených možností (označené žlutě) počítat s určitou bezpečnostní rezervou;<ul>
<li>Předvolby &quot;standardní&quot; a &quot;vysoká&quot; poskytují 14.3% respektive 33.5% redundanci. Vytváření souborů pro opravu chyb pomocí těchto předvoleb je velmi rychlé díky použití optimalizovaného kódu.</li>
<li>Po aktivaci volby &quot;jiná&quot; můžete pomocí posuvníku zvolit libovolnou redundanci.</li>
<li>Pomocí volby &quot;Použít nejvíce&quot; můžete určit velikost souboru pro opravu chyb v MB. dvdisaster vybere takovou redundanci aby se výsledná velikost souboru pro opravu chyb co nejvíce přiblížila, ale nepřesáhla nastavený limit.</li>
</ul>Redundance také určí velikost souboru pro opravu chyb; použití x% redundance vytvoří souboru pro opravu chyb velikostí odpovídající x% velikosti bitové kopie. Použití redundancí nižších než u volby &quot;standardní&quot; (14.3%) není doporučeno, protože pak nelze zaručit dostatečnou ochranu dat. <?php end_screen_shot(); ?><?php begin_screen_shot("Záložka \"Soubory\".","create-prefs-file.png"); ?><b>Záložka &quot;Soubory&quot;.</b> Na této záložce prozatím ponechte nastavení tak jak jsou, návrhy na další <a href="howtos25.php">optimalizaci</a> budou následovat.<?php end_screen_shot(); ?><pre> </pre><b>Nepoužité záložky</b><p>Záložka &quot;Ostatní&quot; prozatím obsahuje pouze funkce pro vytváření souborů se záznamem. Tyto volby jsou užitečné při zasílání <a href="feedback.php">hlášení o chybě</a>, ale při běžném používání by měly být vypnuté. Záložka &quot;Vzhled&quot; umožňuje nastavit barvy používané k zobrazení informací podle vašeho vkusu, ale nemá žádný vliv na tvorbu dat pro opravu chyb.<pre> </pre><a href="howtos23.php">Vytvoření dat pro opravu chyb...</a> <!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>