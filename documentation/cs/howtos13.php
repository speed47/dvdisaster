<?php
# dvdisaster: English homepage translation
# Copyright (C) 2004-2012 Carsten Gnörlich
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()../images/
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

require("../include/dvdisaster.php");
require("../include/screenshot.php");

begin_page();

howto_headline("Kontrola poškození disků", "Interpretace výsledků", "images/scan-icon.png");
?>

<!-- Insert actual page content below -->

<?php begin_screen_shot("Přehled","defective-cd.png"); ?><b>Přehled.</b> dvdisaster poskytuje o průběhu a výsledcích kontroly několik informací:<ul>
<li>Spirála pod &quot;<b>Stav disku</b>&quot; (vpravo).<p>Spirála poskytuje informace o čitelnosti disku. Disk je zcela čitelný pokud jsou všechny bloky zbarveny zeleně. Žluté nebo červené bloky označují místa, kde nebylo možné data úspěšně přečíst. Celkový počet nečitelných sektorů je vypsán ve zprávě <i>&quot;Kontrola dokončena:&quot;</i> ve spodní části okna.<p></li>
<li>&quot;<b>Rychlost</b>&quot; - Křivka zobrazující rychlost čtení (vlevo nahoře).<p>Rychlost čtení není absolutním měřítkem kvality disku, ale je užitečná jako určité vodítko: Čím je křivka pravidelnější, tím je disk v lepším stavu. Příklady křivek rychlosti při čtení bezchybného a poškozeného disku jsou uvedeny na jiném místě této stránky.<p></li>
<li>&quot;<b>C2 chyby</b>&quot; - Měřítko stavu disku poskytované mechanikou (vlevo dole).<p>Tento typ kontroly je <a href="qa.php?pipo">zatím k dispozici pouze pro CD disky</a>. CD mechaniky mají zabudovanou opravu chyb, která dokáže předcházet ztrátě dat způsobené malým poškozením a vadami disku. Počet C2 chyb indikuje, jak často musela mechanika v průběhu testu použít své opravné mechanismy - na nepoškozeném disku by tato hodnota měla být nulová.</li>
</ul>
<?php end_screen_shot(); ?><b>Příklady pro nepoškozený disk</b><p><?php begin_screen_shot("Nepoškozené CD","good-cd.png"); ?> <b>Nepoškozené CD</b>: Tento snímek obrazovky ukazuje nepoškozené CD: Všechny bloky pod &quot;Stav disku&quot; jsou zelené, nebyly hlášeny žádné C2 chyby a křivka rychlosti čtení je hladká. Stoupající křivka rychlosti čtení je pro většinu disků normální (výjimku potvrzující pravidlo uvidíte v následujícím snímku). Malé špičky na začátku a na konci křivky jsou normální; malé odchylky jako ta na pozici 250M jsou také neškodné. <?php end_screen_shot(); ?><?php begin_screen_shot("Nepoškozené dvouvrstvé DVD","good-dvd9.png"); ?><b>Někdy křivka rychlosti čtení nemusí rovnoměrně stoupat</b>: křivky pro vícevrstvé disky mohou symetricky stoupat i klesat. Bez uvedeného příkladu, ale také možné, jsou ploché křivky bez změny rychlosti čtení (typické hlavně pro DVD-RAM).<?php end_screen_shot(); ?><p><b>Příklad lehce poškozeného disku</b><p><?php begin_screen_shot("Mírně poškozené CD","weak-cd.png"); ?>Tento disk je stále čitelný, jak je vidět podle zelené spirály pod &quot;Stav disku&quot;. Ale lze rozpoznat viditelné náznaky budoucích problémů: mechanika musí ke konci disku viditelně zpomalit aby bylo možné z něj načíst data. Všimněte si viditelného zpomalení okolo značky 600M. To je doprovázeno vzrůstem detekovaných C2 chyb k hranici 100 chyb; to je dalším varovným signálem, že dochází ke zhoršování kvality disku u jeho vnějšího okraje. Pokud jste pro tento disk ještě nevytvořili <a href="howtos20.php">data pro opravu chyb</a>, je toto pravděpodobně poslední příležitost kdy tak můžete učinit, protože u takového disku se brzy začnou vyskytovat nečitelné sektory.<?php end_screen_shot(); ?><p><b>Příklady poškozených disků</b><p><?php begin_screen_shot("Poškozené CD","defective-cd.png"); ?> <b>Poškozené CD.</b> Červeně zbarvené sektory ve spirále zobrazují velké nečitelné oblasti na vnější části disku. Ve spodní části okna naleznete informace o tom, že disk obsahuje 28752 nečitelných sektorů. To představuje přibližně 8.2% poškozených sektorů (z 352486 sektorů celkem), což je stále ještě bezpečně v mezích <a href="howtos40.php">obnovitelnosti</a> pomocí <a href="howtos20.php">dat pro opravu chyb (ECC)</a> vytvořených pomocí výchozího nastavení - pokud jste si ovšem tato data včas vytvořili! V opačném případě jsou ale data z těchto červených částí ztracena, protože k vytvoření dat pro obnovu chyb není možné použít již poškozený disk.<?php end_screen_shot(); ?><p><a name="crc"></a> <?php begin_screen_shot("Chyby kontrolního součtu","crc-cd.png"); ?> <b>Chyby kontrolního součtu.</b> Žluté body ve spirále představují místa, která byla čitelná, ale u kterých nesouhlasil kontrolní součet přečtených dat se součtem uloženým v datech pro opravu chyb. Existují dvě hlavní příčiny:<p><ul><li><b>Bitová kopie byla pozměněna</b> po vytvoření dat pro opravu chyb, ale před zapsáním na disk. To se může stát na unixových systémech pokud po vytvoření dat pro obnovu chyb bitovou kopii připojíte s přístupem pro čtení. Typické jsou pak chyby CRC u sektoru 64 a u sektorů 200 až 400, protože systém používá tyto oblasti pro uložení změn v datech přístupu k souborům. Provedení obnovy dat pomocí dvdisaster je v tomto případě bezpečné.<p>Pokud jste však u bitové kopie provedli po vytvoření dat pro opravu chyb změny v souborech, budou taková data pro obnovu chyb jak bezcenná, tak i nebezpečná. Provedení opravy disku obnoví jeho stav do podoby v jaké byl při vytvoření dat pro opravu chyb, což v tomto případě zcela jasně nebude představovat aktuální obsah disku.<p></li>

<li><b>Počítač má technické problémy</b> při komunikace s úložným zařízením. Proveďte kontrolu znovu a sledujte pozici zaznamenaných chyb CRC. Pokud se již chyby CRC neobjeví, nebo mění svoji polohu, váš systém může mí poškozenou RAM, špatné kabely/řadiče nebo chybně nastavené takty.</li></ul>
<?php end_screen_shot(); ?><p><?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>