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

<h3 class="top">Adaptivní strategie čtení</h3>dvdisaster podporuje dvě rozdílné strategie čtení.<p><b>Adaptivní strategie čtení je doporučena pro:</b><p><ul>
<li><a href="howtos42.php">extrakci dat</a> z poškozených disků</li>
</ul><b><a href="qa34.php">Lineární strategie čtení</a> je doporučena pro:</b><p><ul>
<li><a href="howtos23.php">vytváření bitových kopií</a> z nepoškozených disků, například pro vytvoření souborů pro opravu chyb</li>
<li><a href="howtos12.php">kontrolu</a> rychlosti čtení a čitelnosti disků</li>
</ul>

<pre> </pre><b>Vlastnosti adaptivní strategie čtení.</b><p>Adaptivní strategie čtení používá při hledání čitelných částí poškozeného disku přístup &quot;rozděl a panuj&quot;. Strategie je inspirována článkem publikovaným Harald Bögeholz v magazínu c&#39;t 16/2005, který byl publikován společně s programem <i>h2cdimage</i>:<ol>
<li>Na začátku je disk považován za jeden nepřečtený rozsah. Čtení začne od sektoru nula.<p></li>
<li>Čtení pokračuje dokud není dosaženo konce aktuálního rozsahu nebo nedojde k chybě čtení.<p></li>
<li>Čtení je přerušeno pokud (3a) je načteno dostatečné množství sektorů potřebné k provedení opravy chyb nebo (3b) již nejsou k dispozici žádné nečitelné rozsahy přesahující zadanou hraniční velikost.<p></li>
<li>V opačném případě je vyhledán největší dosud nepřečtený rozsah. Čtení pokračuje od poloviny (tedy v druhé polovině) tohoto rozsahu; čtení první poloviny tohoto rozsahu je odloženo a bude provedeno v některém z následujících průchodů čtení.</li>
</ol>

<?php begin_screen_shot("Probíhající adaptivní čtení","adaptive-progress.png"); ?>Podmínka ukončení (3a) je zvláště efektivní: Čtení je přerušeno v okamžiku kdy je načten dostatek dat pro úspěšnou obnovu bitové kopie s využitím souboru pro opravu chyb. To může ve srovnání s pokusem o úplné přečtení zkrátit dobu čtení až o 90 procent, funguje ovšem pouze pokud je k dispozici soubor pro opravu chyb.<p><?php end_screen_shot(); ?><p><pre> </pre><a name="configure"></a> <b>Nastavení</b><p><b>Soubor pro opravu chyb.</b> Adaptivní čtení pracuje nejlépe pokud jsou k dispozici data pro opravu chyb. Data pro opravu chyb musela být samozřejmě <a href="howtos21.php">vytvořena</a> v době, kdy byl disk stále zcela čitelný. Pro použití souboru pro opravu chyb během adaptivního čtení, <a href="howtos42.php#select_eccfile">zadejte jeho název</a> před spuštěním čtení.<p><b>Omezení rozsahu čtení.</b> Čtení může být <a href="howtos11.php#image">omezeno</a> na určitou část disku. Pokud jsou k dispozici data pro opravu chyb, není použití této možnosti doporučeno, protože může bránit v načtení sektorů, které jsou vyžadovány k provedení opravy. Pokud nejsou data pro opravu chyb k dispozici, může být omezení rozsahu čtení užitečné při opakovaných pokusech čtení.<p><b>Včasné ukončení čtení.</b> Pokud nejsou k dispozici data pro opravu chyb, bude adaptivní čtení ukončeno pokud již nezbývají žádné rozsahy <a href="howtos41.php#reading_attempts">větší než je zadaná hraniční velikost</a>.<p>Zadaná hraniční velikost by neměla být nižší než 128. V opačném případě by na konci čtení musela hlavička laseru provádět velké množství změn polohy. To snižuje jak životnost mechaniky, tak její schopnost přesného čtení. Lepším přístupem je ukončit adaptivní čtení dříve a ve čtení zbývajících sektorů pokračovat pomocí <a href="qa34.php">lineárního čtení</a>.<!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>