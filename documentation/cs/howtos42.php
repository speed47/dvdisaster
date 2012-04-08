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

howto_headline("Oprava bitových kopií disků", "Postup", "images/fix-icon.png");
?>

<!-- Insert actual page content below -->Ujistěte se, že byl dvdisaster nastaven tak, jak je uvedeno v sekci <a href="howtos41.php">základní nastavení</a>. Poté postupujte podle následujícího návodu:<p><hr><a name="a"></a><table>
<tr>
<td class="w200x" align="center"><img src="../images/slot-in.png" alt="Ikona: Vložit disk do mechaniky"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů"></td>
<td>  </td>
<td class="valignt"><b>Vložte poškozený disk do mechaniky</b> která je přímo připojena k vašemu počítači. Nelze použít síťové mechaniky, softwarové mechaniky ani mechaniky virtuálních počítačů.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><img src="../images/winbrowser.png" alt="Ikona: Zavřít okna otevřená funkcí Autoplay"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů"></td>
<td>  </td>
<td class="valignt"><b>Zavřete všechna okna</b> která váš operační systém otevřel k procházení obsahu disku a případně ukončete programy které mohli být z daného disku automaticky spuštěny. Počkejte až mechanika disk identifikuje a přestane ho načítat.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><a href="howtosa1.php"> <img src="../images/select-drive.png" alt="Ovládací prvky dvdisaster: Výběr mechaniky (rozevírací nabídka)" class="noborder"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></a></td>
<td>  </td>
<td class="valignt"><b>Vyberte mechaniku</b> obsahující poškozený disk z rozevírací nabídky dvdisaster.</td>
</tr>
</table><a name="select_eccfile"></a><table>
<tr>
<td class="w200x"align="center"><img src="../images/select-ecc.png" alt="Ovládací prvky dvdisaster: Výběr souboru pro opravu chyb (vstupní pole a tlačítko)" class="nobordervalignm"></td>
<td>  </td>
<td class="valignt">Pokud používáte <a href="howtos20.php">soubor pro opravu chyb</a> zadejte jeho název do zobrazeného pole. Pokud byl disk <a href="howtos30.php">rozšířen o data pro opravu chyb</a>, ponechte tuto položku prázdnou.<br></td>
</tr>
<tr>
<td class="w200x" align="center"><a href="howtosa1.php"> <img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></a></td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><a href="howtosa4.php"> <img src="images/read-icon.png" alt="Ovládací prvky dvdisaster: Načíst (tlačítko)" class="noborder"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></a></td>
<td>  </td>
<td class="valignt"><b>Klikněte na tlačítko &quot;Načíst&quot;</b> pro spuštění procesu čtení.</td>
</tr>
</table>

<?php begin_howto_shot("Načtení disku.","adaptive-progress.png", ""); ?><b>Sledujte průběh čtení.</b> Adaptivní strategie čtení provede systematické vyhledání čitelných sektorů. V průběhu čtení bude možné sledovat vytváření mezer, které ale budou v průběhu čtení postupně uzavírány. Tento efekt je obvykle méně výrazný než na zobrazeném příkladu. Pokud jsou všechny poškozené sektory na konci disku, může být proces čtení přerušen ještě před tím než narazí na první nečitelný sektor.<?php end_howto_shot(); ?><p><table>
<tr>
<td class="w200x" align="center"><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></td>
</tr>
</table>

<?php begin_howto_shot("Reading process successful.","adaptive-success.png", ""); ?><b>Váš další postup záleží na výsledku čtení.</b> Čtení je automaticky přerušeno v okamžiku kdy je získáno dostatek dat k opravě disku (viz zeleně označený výstup). V tom případě pokračujte kliknutím na tlačítko &quot;Opravit&quot;. <?php end_howto_shot(); ?> <?php begin_howto_shot("neúspěšné čtení.","adaptive-failure.png", ""); ?>Čtení bude také přerušeno pokud se nepodařilo nalézt dostatek čitelných sektorů (viz výstup označený červeně). Bitová kopie v tomto nekompletním stavu ještě <b>není</b> opravitelná. Pokuste se získat dodatečná data za pomoci tipů uvedených v <a href="howtos43.php">pokročilém nastavení</a>.<?php end_howto_shot(); ?><table>
<tr>
<td class="w200x" align="center"><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></td>
<td></td><td></td>
</tr>

<tr>
<td class="w200x" align="center"><a name="b"></a><a href="howtosa4.php"> <img src="images/fix-icon.png" alt="Ovládací prvky dvdisaster: Opravit (tlačítko)" class="noborder"></a></td>
<td>  </td>
<td class="valignt">Pro spuštění <b>opravy bitové kopie</b> klikněte na tlačítko Opravit (funguje <b>pouze</b> pokud byl proces čtení úspěšný!).</td>
</tr>

<tr>
<td class="w200x" align="center"><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></td>
<td></td><td></td>
</tr>
</table>

<?php begin_howto_shot("Sledujte průběh opravy.","fix-success.png", ""); ?><b>Sledujte průběh opravy.</b> Adaptivní čtení je vždy automaticky přerušeno v okamžiku kdy je získáno dostatek dat k opravě, zatížení opravy chyb bude proto vždy maximální. To má za následek zobrazení masivního červeného bloku v grafu &quot;Chyby/ECC blok&quot; a není důvodem k obavám. V závislosti na velikosti disku a výkonu vašeho systému může oprava trvat několik minut až hodin.<?php end_howto_shot(); ?><table>
<tr>
<td class="w200x"align="center"><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></td>
</tr>
</table>

<table>
<tr>
<td class="w200x"align="center"><img src="../images/good-image.png" alt="Ikona: Kompletní bitová kopie" class="nobordervalignm"></td>
<td>  </td>
<td class="valignt">Po dokončení opravy budou všechna data bitové kopie opět kompletní.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x"align="center"><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><a href="howtos33.php?way=2#c"> <img src="thumbnails/write-iso1.png" alt="Ikona: Zápis bitové kopie na disk" class="noborder"></a></td>
<td>  </td>
<td class="valignt"><b>Vypalte opravenou bitovou kopii</b> na nový disk. Postupujte podle stejných instrukcí jaké jsou uvedeny pro <a href="howtos33.php?way=2#c">vypálení disku</a> který byl <a href="howtos33.php">rozšířen o data pro opravu chyb</a>.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x"align="center"><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></td>
</tr>
</table>

<table>
<tr>
<td class="w200x"align="center"><img src="../images/old-cd.png" alt="Ikona: Starý (poškozený) disk" class="nobordervalignm"> <img src="../images/old-image.png" alt="Ikona: Starý soubor bitové kopie" class="nobordervalignm"> <img src="../images/good-cd.png" alt="Ikona: Nepoškozený disk (bez chyb)" class="nobordervalignm"></td>
<td>  </td>
<td class="valignt">Nyní jste vytvořili nový disk obsahující opravená data. Nezapomeňte <a href="howtos10.php">zkontrolovat zda je zcela čitelný</a>. Pak můžete zahodit poškozený disk a smazat vytvořenou bitovou kopii. Pokud jste však pro původní disk vytvořili soubor pro opravu chyb, můžete ho použít i pro nově vytvořený opravený disk.</td>
</tr>
</table>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
