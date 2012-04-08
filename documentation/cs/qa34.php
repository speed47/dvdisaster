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

<h3 class="top">Lineární strategie čtení</h3>dvdisaster podporuje dvě rozdílné strategie čtení.<p><b>Lineární strategie čtení je doporučena pro:</b><p><ul>
<li><a href="howtos23.php">vytváření bitových kopií</a> z nepoškozených disků, například pro vytvoření souborů pro opravu chyb</li>
<li><a href="howtos12.php">kontrolu</a> rychlosti čtení a čitelnosti disků</li>
</ul><b><a href="qa35.php">Adaptivní strategie čtení</a> je doporučena pro:</b><p><ul>
<li><a href="howtos42.php">extrakci dat</a> z poškozených disků</li>
</ul>

<pre> </pre><b>Vlastnosti lineární strategie čtení.</b><p><?php begin_screen_shot("Mírně poškozené CD","weak-cd.png"); ?>Optické disky jsou rozděleny do sektorů průběžně číslovaných od nuly. Každý sektor obsahuje 2048 bajtů dat.<p>Lineární strategie čtení čte disk od začátku (sektor 0) do konce (poslední sektor). Rychlost čtení je graficky zobrazena a její průběh může napovědět o <a href="#quality">kvalitě disku</a>:<?php end_screen_shot(); ?><p><pre> </pre><a name="configure"></a> <b>Nastavení.</b><p><b>Počet sektorů přeskočených po chybě čtení.</b> Čtení poškozených sektorů je pomalé a při nepříznivých podmínkách může vést k opotřebení mechaniky. Výskyt několik chyb v jednom spojitém rozsahu sektorů je mnohem častější než výskyt samostatné chyby. Existuje proto <a href="howtos11.php#read_attempts"> volba</a> umožňující nastavení určitého počtu sektorů, které budou přeskočeny pokud dojde k chybě čtení. Přeskočené sektory jsou bez jakýchkoliv pokusů o čtení považovány za nečitelné. Tipy pro nastavení počtu přeskočených sektorů:<p><ul>
<li>Přeskočení většího počtu sektorů (např. <b>1024</b>) poskytne rychlý přehled o poškozených oblastech, neumožní však obvykle získat dostatečné množství dat pro opravu bitové kopie.<p></li> 
<li>Menší hodnoty jako <b>16, 32 nebo 64</b> jsou vhodným kompromisem: Čas zpracování bude výrazně zkrácen, ale přesto bude získáno dostatek dat pro obnovu bitové kopie.<p></li>
</ul>Na DVD discích zabírají chyby čtení z technických důvodů obvykle minimálně 16 sektorů. Přeskakování nižšího počtu sektorů než 16 proto není u DVD doporučeno.<p><a name="range"></a> <b>Omezení rozsahu čtení.</b> V nastavení na záložce &quot;Bitová kopie&quot; může být čtení <a href="howtos11.php#image">omezeno na určitý rozsah sektorů</a>. To se hodí v případě opakovaných čtení poškozených disků.<pre> </pre><a name="quality"></a> <b>Odhad kvality disku.</b><p><a name="error"></a> <b>Křivka rychlosti čtení.</b> Většina mechanik při čtení oblastí s horší kvalitou zpomalí:<ul>
<li>Poklesy v křivce rychlosti čtení mohou být varováním před blížícím se selháním disku.</li>
<li>Některé mechaniky však i při čtení poškozených oblastí stále čtou maximální rychlostí. U těchto mechaniky se zhoršení kvality disku v křivce rychlosti čtení neprojeví dokud opravdu nedojde k chybě čtení.</li>
</ul><p>Křivka čtení je nejpřesnější při použití funkce <a href="howtos10.php"> &quot;Zkontrolovat&quot;</a>. Při provádění funkce <a href="howtos23.php">&quot;Načíst&quot;</a> jsou současně se čtením ukládána data na pevný disk, což může v křivce čtení při určité kombinaci operačního systému a hardwaru způsobovat nepravidelnosti.<p><b>Chyby čtení.</b> Chyby čtení jsou zobrazeny jako <a href="howtos13.php#defective">červené bloky ve spirále</a> znázorňující disk nebo odpovídající varovná hlášení v příkazovém řádku. To znamená, že disk nebylo možné v těchto místech přečíst:<ul>
<li>Disk je z největší pravděpodobností poškozen.</li>
<li>Bitová kopie by měla být co nejdříve <a href="howtos40.php">opravena</a> a poté vypálena na nový disk.</li>
</ul>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
