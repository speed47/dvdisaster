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

<h3 class="top">Chybová hlášení</h3><a href="#tao">3.1 &quot;Varování: na konci disku chybí 2 sektory&quot;.</a><p><a href="#block">3.2 Program zatuhne ihned po spuštění.</a><p><a href="#crc">3.3 Co znamená &quot;Chyba CRC, sektor: n&quot;?</a><p><a href="#rw">3.4 Chyby čtení a špatná velikost bitové kopie s disky -RW/+RW/-RAM</a><p><a href="#dvdrom">3.5 Vypálený disk je detekován jako &quot;DVD-ROM&quot; a je odmítnut.</a><p><a href="#freebsd">3.6 Pod FreeBSD nejsou k dispozici žádné mechaniky.</a><p><a href="#v40error">3.7 &quot;Soubor ECC byl vytvořen ve verzi 0.40.7.&quot;</a><p><hr><p><b><a name="tao">3.1 &quot;Varování: na konci disku chybí 2 sektory&quot;.</a></b><p>Toto varování se objevuje u CD disků zapsaných v režimu &quot;TAO&quot; (track at once). Některé mechaniky u takovýchto disků hlásí o 2 sektory větší velikost, což má za následek 2 chyby čtení na konci disku, které ale <i>neznamenají</i> poškození dat.<p>Protože použitý režim zápisu nelze z disku poznat, předpokládá dvdisaster, že jde o &quot;TAO&quot; CD, pokud jsou na konci disku přesně dva nečitelné sektory a bitovou kopii odpovídajícím způsobem zkrátí. Je na vás abyste rozhodli, zda je to tak správně. dvdisaster můžete přinutit považovat tyto sektory za chyby čtení použitím volby --dao nebo nastavením příslušné volby v nastavení čtení/kontroly.<p>Pokud chcete podobným problémům předejít, používejte režim &quot;DAO / Disc at once&quot; (někdy také nazývaný &quot;SAO / Session at once&quot;).<div class="talignr"><a href="#top">↑</a></div><b><a name="block">3.2 Program zatuhne ihned po spuštění.</a></b><p>Při použití starých verzích jádra Linuxu (2.4.x) program občas zatuhne ihned po spuštění, ještě před tím, než jsou provedeny jakékoliv akce. Nemůže pak být ukončen ani pomocí Ctrl-C nebo &quot;kill -9&quot;.<p>Pokud chcete program ukončit, vysuňte nejdříve disky z mechanik. Znovu načtěte disk a počkejte na dokončení jeho rozpoznání a zpomalení otáček mechaniky. Po opětovném spuštění by pak již dvdisaster měl fungovat.<div class="talignr"><a href="#top">↑</a></div><b><a name="crc">3.3 Co znamená &quot;Chyba CRC, sektor: n&quot;?</a></b><p>Daný sektor se podařilo přečíst, ale jeho kontrolní součet neodpovídá hodnotě uložené v souboru pro opravu chyb. Možné příčiny jsou:<p><ul>
<li>Bitová kopie byla načtena s možností zápisu a byla proto změněna (typické příznaky: chyby CRC sektoru 64 a v sektorech 200 až 400).</li>
<li>Máte problémy s hardwarem, zvláště pak při komunikaci s úložnými zařízeními.</li>
</ul>Pokud máte podezření na technické problémy, zkuste znovu vytvořit bitovou kopii a soubor pro opravu chyb a <a href="howtos50.php">znovu proveďte ověření</a>. Pokud chyba zmizí, nebo se objeví na jiném místě, může to být v důsledku vadné paměti, poškozeného kabelu mechaniky, nebo špatným nastavením frekvence procesoru/čipsetu.<div class="talignr"><a href="#top">↑</a></div><b><a name="rw">3.4 Chyby čtení a špatná velikost bitové kopie s disky -RW/+RW/-RAM</a></b><p>Některé mechaniky u disků -RW/+RW/-RAM hlásí chybnou velikost bitové kopie. Dva časté případy jsou:<p><table>
<tr><td class="valignt">Problém:</td>
<td>Mechanika hlásí velikost největší bitové kopie, jaký byl kdy na disk vypálen, ne velikost aktuální bitové kopie.</td></tr>
<tr><td class="valignt">Příznaky:</td>
<td>Po smazání je na disk vypálen soubor velikosti 100MB. Bitová kopie vytvořená z tohoto disku má ale velikost několik GB a obsahuje zbytky dříve vypálených bitových kopií.</td></tr>
<tr><td><pre> </pre></td><td></td></tr>
<tr><td class="valignt">Problém:</td>
<td>Mechanika hlásí místo velikosti aktuální bitové kopie velikost maximální kapacity disku (obvykle 2295104 sektorů).</td></tr>
<tr><td class="valignt">Příznaky:</td>
<td>Po dosažení určitého bodu jsou již při čtení hlášeny pouze chyby čtení, ale všechny soubory jsou čitelné a kompletní.</td></tr>
</table><p>Možné řešení:<p><table width="100%"><tr><td class="vsep"></td><td>Aktivujte možnost určení velikosti bitové kopie ze systému souborů ISO/UDF nebo z ECC/RS02 dat.</td></tr></table><p>Pokud nejsou potřebné sektory ISO/UDF čitelné a používáte soubor pro opravu chyb existují dvě možnosti řešení:<ul>
<li>Spusťte funkci <a href="howtos50.php">&quot;Ověření&quot;</a> a vyberte pouze soubor pro opravu chyb. Poznamenejte si uvedenou správnou velikost bitové kopie a omezte podle ní rozsah čtení.</li>
<li>Načtěte bitovou kopii s nesprávnou (větší) velikostí. Při vyvolání funkce <a href="howtos40.php#repair">&quot;Opravy&quot;</a> odpovězte&quot;OK&quot; při dotazu zda má být bitová kopie zkrácena.</li>
</ul>

<div class="talignr"><a href="#top">↑</a></div><b><a name="dvdrom">3.5 Vypálený disk je detekován jako &quot;DVD-ROM&quot; a je odmítnut.</a></b><p>Book type disku byl nastaven na &quot;DVD-ROM&quot;. Pro zpracování takových disků pomocí dvdisaster je většinou potřeba mechanika schopná takovéto disky zapisovat.<p>Např. dvouvrstvá DVD+R s chybným booktype budou přijata pouze při čtení ve vypalovačce, která je schopná takové disky vypálit.<p>Zkuste v tomto případě použít jinou mechaniku.<div class="talignr"><a href="#top">↑</a></div><b><a name="freebsd">3.6 Pod FreeBSD nejsou k dispozici žádné mechaniky.</a></b><p><ul>
<li>FreeBSD může ke zprovoznění ATAPI mechanik (téměř všechny současné modely) v dvdisaster vyžadovat <a href="download10.php#freebsd">překompilování jádra</a>.<li>Pro dané zařízení (např. /dev/pass0) jsou potřebná oprávnění jak pro čtení, tak pro zápis.</ul>

<div class="talignr"><a href="#top">↑</a></div><b><a name="v40error">3.7 &quot;Soubor ECC byl vytvořen ve verzi 0.40.7.&quot;</a></b><p><a href="http://sourceforge.net/cvs/?group_id=157550">CVS verze</a> dvdisaster označují ECC soubory speciálním příznakem. To způsobuje do verze dvdisaster 0.65 chybné zobrazení výše uvedeného chybového hlášení. Používejte CVS verze pouze s dvdisaster 0.66 nebo novějšími.<div class="talignr"><a href="#top">↑</a></div>


<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
