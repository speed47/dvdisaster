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

<h3 class="top">Příklady</h3>Na předchozí stránce jste si mohli prohlédnout příklady <a href="howtos52.php?expand=1">nepoškozených bitových kopií a dat pro opravu chyb</a> a <a href="howtos53.php?expand=1">nepoškozených bitových kopií rozšířených o data pro opravu chyb</a>. V následujícím textu pak budou uvedeny některé chybové situace:<p><hr>Dva typické příklady zobrazující informace bitových kopií, které ještě nebyly zcela opraveny:<p><?php begin_screen_shot("Bitová kopie s nečitelnými sektory, RS01","compare-bad-rs01.png"); ?> <b>Bitová kopie s nečitelnými sektory a souborem pro opravu chyb</b>. Zobrazená bitová kopie obsahuje 6245 nečitelných sektorů; data pro opravu chyb jsou k dispozici jako soubor pro opravu chyb.<p><?php end_screen_shot(); ?> <?php begin_screen_shot("Bitová kopie s nečitelnými sektory, RS02","compare-bad-rs02.png"); ?> <b>Bitová kopie rozšířená o data pro opravu chyb obsahující nečitelné sektory</b>. Tato bitová kopie obsahuje poblíž svého konce nečitelné sektory. Zasažena je především ECC sekce, protože data pro opravu chyb jsou umístěna na konci bitové kopie. Upozorňujeme, že i přesto nedochází k oslabení schopnosti opravy chyb protože je nezávislá na poloze: oprava 10000 chyb na začátku disku je stejně snadná jako oprava 10000 chyb na jeho konci.<br>RS02 enkodér použitý k vytvoření dat pro opravu chyb je schopný odhadnout šanci na úspěšnou opravu chyb bitové kopie. Tato informace je zobrazena na konci výpisu informací o datech pro opravu chyb.<?php end_screen_shot(); ?><hr>  

<?php begin_screen_shot("Přerušené načítání","compare-trunc-rs01.png"); ?><b>Bitová kopie z přerušeného načítání</b>. Bitová kopie je kratší než bylo očekáváno; k tomu obvykle dochází, pokud byl předčasně ukončen proces načítání.<p><?php end_screen_shot(); ?> <?php begin_screen_shot("Bitová kopie je příliš velká","compat-150-rs01.png"); ?> <b>Bitová kopie je větší než bylo očekáváno</b>. Bitová kopie je větší než bylo očekáváno; možné důvody jsou popsány v sekci o <a href="howtos90.php">kompatibilitě bitových kopií</a>. Tento problém je možné v některých případech vyřešit; prohlédněte si tipy týkající se <a href="howtos91.php#err">používání souborů pro opravu chyb</a> a <a href="howtos92.php#err">používání rozšířených bitových kopií</a>. <?php end_screen_shot(); ?> <?php begin_screen_shot("Špatný soubor pro opravu chyb","compare-mismatch-rs01.png"); ?><b>Špatný soubor pro opravu chyb</b>. Soubor pro opravu chyb patří k jiné bitové kopii. To způsobuje spoustu CRC chyb, protože sektory obsahují zcela jiná data. Nejpodstatnějším znakem ale je:<p>Otisk: <span class="red">nesouhlasí</span><p>Z toho je zřejmé, že soubor pro opravu chyb patří k jiné bitové kopii.<?php end_screen_shot(); ?> <!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>