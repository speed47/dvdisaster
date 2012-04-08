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

<h3 class="top">Poznámky k chybám čtení</h3>Optické disky obsahují vlastní opravné kódy chránící data před malými chybami při výrobě a nepřesnostmi při vypalování. Pokud jsou mechanika a disk kompatibilní a kvalitní, nebude muset být oprava chyb integrovaná na disku vůbec použita. Díky tomu je kapacita vestavěné ochrany dostatečná ke kompenzaci běžného opotřebení a stárnutí během příštích pár let používání disku.<p>Po překročení kapacity vestavěné opravy chyb začne u disku docházet k výskytu chyb čtení. Ty mohou být v dvdisaster zjištěny prostřednictvím funkce <a href="howtos10.php">&quot;Zkontrolovat&quot;</a>. V závislosti na době prvního výskytu jsou nejvíce zajímavé dva typy chyb čtení:<p><b>Chyby čtení ihned po vypálení disku.</b> Tyto jsou známkou:<ul>
<li>disku z vadné série, nebo</li>
<li>disku nekompatibilního s vypalovací mechanikou.</li>
</ul>Obezřetnou volbou je poškozený disk zahodit a zapsat data na nový nepoškozený disk, případně i změna výrobce použitých disků.<p>Odolejte prosím pokušení uchovat poškozená disk s pomocí souboru pro opravu chyb - s největší pravděpodobností bude výsledkem ztráta dat.<p><b>Chyby čtení po několika měsících/letech.</b> Vestavěná oprava chyb disku bude během jeho životnosti stále více zatížena až nakonec selže a dojde k výskytu chyb čtení. Důvod může být jak mechanický (škrábance, deformace disku), tak chemický (rozpad barviv a/nebo odrazivé vrstvy).<p>Tento proces je postupný a dochází k němu, i když jsou disky po určitou dobu pouze skladovány a v jeho důsledku nemusí být možné přečíst všechny sektory.<p>Je proto důležité vytvořit <a href="howtos21.php">data pro opravu chyb</a> včas. ECC data obsahují informace pro dopočet obsahu chybějících sektorů <a href="qa31.php">(v určitých mezích)</a>. Díky ECC datům tak dvdisaster může opravit bitové kopie, i když se nepodařilo přečíst všechny sektory.<p>Protože oprava chyb může obnovit je určitý počet chybějících sektorů, není nutné pokoušet se z disku získat všechny čitelné sektory. <a href="qa35.php">Adaptivní strategie čtení</a> během čtení kontroluje, zda již byl načten dostatek dat k opravě bitové kopie. Jakmile je této hranice dosaženo, čtení je zastaveno a nepřečtené sektory jsou dopočítány s využitím ECC dat.<p><a name="reading-tips"><b>Tipy pro efektivní čtení poškozeného disku</b></a><p>Výsledek čtení poškozeného disku závisí na několika faktorech:<ul>
<li><b>Ne všechny mechaniky čtou stejně.</b><br> Různé mechaniky mají různé schopnosti čtení. Využijte funkci dvdisaster pro doplňování bitových kopií z několika pokusů o přečtení a použijte při jednotlivých pokusech rozdílné mechaniky. Pokuste se bitovou kopii doplnit na různých počítačích s různými mechanikami, k přenosu bitové kopie využijte síť nebo přenosná datová média.<p></li>
<li><b>Vysuňte a znovu vložte disk.</b><br> Někdy pomůže disk před dalším pokusem o přečtení vysunou, pootočit (třeba o jednu čtvrtinu) a znovu načíst.<p></li>
<li><b>Některé mechaniky čtou lépe pokud jsou studené.</b><br> Vypněte počítač, nechte ho stát přes noc a další pokus o čtení opakujte ráno.<p>Poznámka: &quot;Studené&quot; znamená běžnou teplotu okolí - vkládání mechaniky nebo disků například do lednice může být velmi nebezpečné.<p></li>
</ul>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
