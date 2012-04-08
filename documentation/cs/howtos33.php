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
$way=$_GET["way"];
if($way & 1) $make_iso_action=$way&2;
else         $make_iso_action=$way|1;
if($way & 2) $write_iso_action=$way&1;
else	     $write_iso_action=$way|2;
?>

<!-- Insert actual page content below -->

<?php
howto_headline("Rozšíření obrazu o data pro opravu chyb", "Postup", "images/create-icon.png");
?>dvdisaster je určen k práci s daty pro opravu chyb a pro čtení poškozených disků. Vytváření bitových kopií ve formátu ISO nebo UDF a jejich zápis na disk je něco úplně jiného a je také velice komplexní. Nemáme zájem v dvdisaster znovu vytvářet kód pro vypalování disků. Místo toho použijeme vypalovací program který běžně používáte s vaší vypalovačkou.<p><pre> </pre><a name="a"></a><table>
<tr>
<td class="w200x" align="center"><?php
echo "<a href=\"howtos33.php?way=$make_iso_action\">\n";
?> <img src="thumbnails/make-iso1.png" alt="Ikona: Vytvoření bitové kopie" class="noborder"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></a></td>
<td>  </td>
<td class="valignt">Pomocí svého vypalovacího programu <b>nejdříve vytvořte bitovou kopii ve formátu ISO</b>. Vyberte soubory, které chcete vypálit, ale zatím nespouštějte vypalování. Místo toho vytvořte bitovou kopii ISO na vašem pevném disku. můžete zobrazit kliknutím na uvedený obrázek.<?php
echo "<a href=\"howtos33.php?way=$make_iso_action\">\n";
echo "podrobný postup</a>.\n";
?></td>
</tr>
</table>

<?php
if($way&1)
{
?>

<hr><b>Příklad: Vytvoření bitové kopie na pevném disku</b>. Protože existuje spousta různých vypalovacích programů, budeme postup demonstrovat na příkladu populárního vypalovacího programu pro GNU/Linux <i>K3b</i>. Pokud používáte jiný program, měli byste přesto být schopni požadované kroky zjistit z následujícího popisu a přizpůsobit je danému programu.<p><hr>

<?php begin_screen_shot("Vytvoření nového projektu","make-iso1.png"); ?><b>Vytvořte nový projekt.</b> Otevřete váš program pro vypalování. Většina programů na začátku předpokládá/vyžaduje vytvoření nového projektu. V projektu pak nastavíte volby pro nový disk a vyberete požadované soubory.<p>Postup v K3b: <i>Vytvořte nový projekt pomocí zvýrazněné volby (&quot;New Data CD project&quot;) v hlavním okně.</i> <?php end_screen_shot(); ?><hr>

<?php begin_screen_shot("Výběr souborů","make-iso2.png"); ?><b>Vyberte soubory které chcete na disk vypálit.</b> Většinou je k dispozici dialog ve kterém můžete zvolit soubory, nebo ze kterého můžete soubory přetáhnout do projektu.<p>Postup v K3b: Vyberte požadované soubory v horní polovině okna. V příkladu byly pro zapsání na disk vybrány soubory <i>backup.tar.gz</i>, <i>win.zip</i> a <i>work.tar.gz</i>. Vybrané soubory jsou zobrazeny ve spodní polovině okna.<p><b>Důležité:</b> Na disku musí zůstat volné místo. Ponechte na disku pro data pro opravu chyb nejméně 20% volného místa.<p>Postup v K3b: <i> Aktuálně obsazený prostor disku je zobrazen pomocí zeleného pruhu ve spodní části okna (558,9 MB).</i> <?php end_screen_shot(); ?><hr>

<?php begin_screen_shot("Nastavení vypalovacího programu","make-iso2.png"); ?><b>Nastavení vypalovacího programu.</b> Program vám před spuštěním vypalování nabídne cíl zápisu. <b>NESMÍTE</b> vybrat CD/DVD/BD vypalovačku, místo toho vyberte vytvoření ISO/UDF na pevném disku.<p><b>Typ:</b> Než budete pokračovat, vyjměte z mechanik všechny zapisovatelné disky, abyste předešli nechtěnému spuštění vypalování.<p>Postup v K3b: <i>Otevřete dialog vypalování pomocí tlačítka &quot;Vypálit&quot; poblíž levého okraje okna.</i> <?php end_screen_shot(); ?> <?php begin_screen_shot("Výběr vytvoření bitové kopie","make-iso3.png"); ?> <b>Výběr zápisu bitové kopie.</b> Většina vypalovacích programů vám jednoduše nabídne možnost vytvoření bitové kopie ve formátu ISO na pevném disku. Pokud se zdá, že váš program tuto možnost postrádá, možná je třeba vybrat místo vypalovačky virtuální &quot;vypalovačku bitových kopií&quot;.<p>Postup v K3b: <i>Vyberte záložku &quot;Zápis&quot;&quot;. Označte možnost &quot;Pouze vytvořit obraz&quot; (označeno zeleně).</i> <?php end_screen_shot(); ?> <?php begin_screen_shot("Výběr souboru bitové kopie","make-iso4.png"); ?> <b>Vyberte soubor bitové kopie a její typ.</b> Vyberte cílový adresář, název a typ bitové kopie. Použijte jen bitové kopie formátu ISO nebo UDF! Ostatní formáty bitových kopií, jako například NRG, dvdisaster nepodporuje; jejich rozšíření v dvdisaster je bez jakéhokoliv varování poškodí a učiní je nepoužitelné.<p>Postup v K3b: <i>Vyberte záložku &quot;Obraz&quot;. Zadejte cílový adresář pro bitovou kopii (ukázkový soubor &quot;medium.iso&quot; bude umístěn v podadresáři &quot;/var/tmp/cg&quot;). K3b vždy vytváří bitové kopie ve formátu ISO, takže nejsou k dispozici žádné volby typu bitové kopie.</i> <?php end_screen_shot(); ?><hr>

<table>
<tr>
<td class="w200x" align="center"><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></td>
<td>  </td>
<td class="valignt"></td>
</tr>
</table>

<?php
}  /* end from if($way&1) above */
?><a name="b"></a><table>
<tr>
<td class="w200x" align="center"><img src="../images/good-image.png" alt="Ikona: Kompletní bitová kopie (z nepoškozeného disku)" class="noborder"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></td>
<td>  </td>
<td class="valignt">Až budete mít bitovou kopii připravenu, <b>otevřete dvdisaster</b>. Ujistěte se, že byl nastaven tak, jak je uvedeno v <a href="howtos32.php">základním nastavení</a>.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><a href="howtosa2.php"> <img src="../images/select-image2.png" alt="Ovládací prvky dvdisaster: Výběr souboru bitové kopie (vstupní pole a tlačítko)" class="noborder"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></a></td>
<td>  </td>
<td class="valignt"><b>Vyberte adresář a název</b> vytvořeného ISO souboru.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><a href="howtosa4.php"> <img src="images/create-icon.png" alt="Ovládací prvky dvdisaster: Vytvořit (tlačítko)" class="noborder"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></a></td>
<td>  </td>
<td class="valignt"><b>Rozšiřte bitovou kopii o data pro opravu chyb</b> pomocí tlačítka &quot;Vytvořit&quot;.</td>
</tr>
</table>

<?php begin_howto_shot("Vytvoření dat pro opravu chyb.","make-ecc1.png", "down-arrow.png"); ?><b>Počkejte na vytvoření dat pro opravu chyb.</b> To může v závislosti na velikosti bitové kopie a volného prostoru na disku chvíli trvat. Zpracování bitové kopie DVD s 20-30% volného prostoru by mělo na současném hardwaru trvat 5-10 minut. <?php end_howto_shot(); ?><?php begin_howto_shot("Porovnání velikosti bitových kopií.","make-ecc2.png", "down-arrow.png"); ?><b>Pozor:</b> dvdisaster nevytváří novou bitovou kopii, ale rozšíří existující. Podívejte se na bitovou kopii ve správci souborů před a po rozšíření pomocí dvdisaster a všimněte si, jak se zvětšila její velikost.<?php end_howto_shot(); ?> <a name="c"></a><table>
<tr>
<td class="w200x" align="center"><?php
echo "<a href=\"howtos33.php?way=$write_iso_action\">\n";
?> <img src="thumbnails/write-iso1.png" alt="Ikona: Zápis bitové kopie na disk" class="noborder"></a></td>
<td>  </td>
<td class="valignt"><b>Vypalte rozšířenou bitovou kopii</b> na disk. Vyberte rozšířenou bitovou kopii ve vašem vypalovacím programu a spusťte vypalování. klikněte na uvedený obrázek.<?php
echo "<a href=\"howtos33.php?way=$write_iso_action\">\n";
echo "příklad</a>.\n";
?></td>
</tr>
</table>

<?php
if($way&2)
{
?>
<table>
<tr>
<td class="w200x" align="center"><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></td>
<td>  </td>
<td class="valignt"></td>
</tr>
</table>

<hr><b>Příklad: Vypálení bitové kopie na disk</b>.<p><?php begin_screen_shot("Výběr vypálení bitové kopie","write-iso1.png"); ?> <b>Výběr režimu vypálení bitové kopie.</b> Znovu otevřete váš program pro vypalování. Vyberte režim vypalování existujících bitových kopií na disk.<p>Postup v K3b: <i>Klikněte na označené volby (&quot;Vypálit obraz CD&quot;) v hlavním okně.</i> <?php end_screen_shot(); ?><hr>

<?php begin_screen_shot("Výběr bitové kopie","write-iso2.png"); ?><b>Výběr bitové kopie.</b> Vyberte bitovou kopii kterou jste právě vytvořili a rozšířili pomocí dvdisaster.<p>Postup v K3b: <i>V zeleně označeném poli zadejte název bitové kopie, nebo bitovou kopii vyberte pomocí dialogu pro výběr souboru (tlačítko s ikonou adresáře).</i><?php end_screen_shot(); ?><?php begin_screen_shot("Ostatní nastavení","write-iso2.png"); ?><b>Ostatní nastavení.</b> Vyberte režim zápisu &quot;DAO&quot; (&quot;disk at once&quot;) pokud to vaše mechanika podporuje. Tato volba zlepší kompatibilitu disku a dat pro opravu chyb. Také tak zabráníte nechtěnému přidání další session, což by data pro opravu chyb zničilo.<p>Postup v K3b: <i>Ve žlutě označeném poli vyberte &quot;DAO&quot;.</i><?php end_screen_shot(); ?> <?php begin_screen_shot("Vypálení disku","write-iso3.png"); ?> <b>Vypálení disku.</b> Spusťte proces vypalování.<p>Postup v K3b: <i>Klikněte na tlačítko &quot;Spustit&quot; v hlavním okně z předchozího snímku obrazovky.</i> <?php end_screen_shot(); ?><hr>

<?php
}  /* end from if($way$2) above */
?>

<table>
<tr>
<td class="w200x" align="center"><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></td>
<td>  </td>
<td class="valignt"></td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><img src="../images/good-cd-ecc.png" alt="Ikona: Nepoškozený disk s daty pro opravu chyb" class="noborder"></td>
<td>  </td>
<td class="valignt"><b>Hotovo!</b> Právě jste vytvořili CD chráněné daty pro opravu chyb.</td>
</tr>
</table>

<pre> </pre><b>Související informace</b><ul>
<li><a href="howtos90.php">Zkontrolujte, zda proces vypalování neovlivnil data pro opravu chyb.</a><p>Je doporučeno tuto kontrolu provést vždy, když použijete novou verzi vašeho vypalovacího programu (nebo zcela jiný vypalovací program), aby bylo jisté, že s dvdisaster bez problémů spolupracuje.</li>
</ul>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
