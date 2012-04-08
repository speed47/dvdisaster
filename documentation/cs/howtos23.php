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
switch($way)
{  case 1: $action="z disku"; break;
   case 2: $action="z bitové kopie ISO"; break;
   default: $action="Postup"; break;
}

howto_headline("Vytvoření souborů pro opravu chyb", $action, "images/create-icon.png");
?>

<!-- Insert actual page content below -->Ujistěte se, že byl dvdisaster nastaven tak, jak je uvedeno v sekci <a href="howtos22.php">základní nastavení</a>, protože použití některých voleb může mít za následek vytvoření ne úplně ideálních dat pro opravu chyb.<p>Další kroky závisí na zdroji pro data pro opravu chyb. Vyberte jeden ze dvou postupů:<p><table width="100%" cellspacing="5">
<tr>

<?php
$expand=$_GET["expand"];
if($expand=="") $expand=0;
echo "<td><a href=\"howtos23.php?way=1&expand=$expand\"><img src=\"../images/good-cd.png\" alt=\"Icon: Good medium (without read errors)\" class=\"noborder\"></a></td>\n";
echo "<td><a href=\"howtos23.php?way=1&expand=$expand\">Vytvoření souboru pro opravu chyb z CD/DVD/BD disku</a></td>\n";
echo "<td><a href=\"howtos23.php?way=2&expand=$expand\"><img src=\"../images/good-image.png\" alt=\"Icon: Complete image\" class=\"noborder\"></a></td>\n";
echo "<td><a href=\"howtos23.php?way=2&expand=$expand\">Vytvoření souboru pro opravu chyb z bitové kopie ISO</a></td>\n";
?>

</tr>
</table>

<?php
if($way==1){
?>
<hr><p><table>
<tr>
<td class="w200x" align="center"><img src="../images/slot-in.png" alt="Ikona: Vložit disk do mechaniky"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů"></td>
<td>  </td>
<td class="valignt"><b>Vložte disk který chcete použít do mechaniky</b> připojené přímo k vašemu počítači. Nelze použít síťové mechaniky, softwarové mechaniky ani mechaniky virtuálních počítačů.</td>
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
<td class="valignt">V rozevírací nabídce dvdisaster <b>vyberte mechaniku, která obsahuje kontrolovaný disk</b>.</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><a href="howtosa2.php"> <img src="../images/select-image.png" alt="Ovládací prvky dvdisaster: Výběr souboru bitové kopie (vstupní pole a tlačítko)" class="noborder"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></a></td>
<td>  </td>
<td class="valignt"><b>Vyberte adresář a název souboru</b> pro ukládanou bitovou kopii.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><a href="howtosa4.php"> <img src="images/read-icon.png" alt="Ovládací prvky dvdisaster: Načíst (tlačítko)" class="noborder"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></a></td>
<td>  </td>
<td class="valignt">Pomocí tlačítka Načíst z vybraného disku <b>vytvořte bitovou kopii</b>.</td>
</tr>
</table>

<?php begin_howto_shot("Načtení bitové kopie.","watch-read1.png", "down-arrow.png"); ?><b>Sledujte proces načítání.</b> Počkejte než je disk zcela načten. Pokud se ukáže, že disk obsahuje poškozené sektory, vytvoření dat pro opravu chyb nebude možné.<?php end_howto_shot(); 
 }  /* end of if($way == 1) */

if($way == 2) {
?><hr><p><table>
<tr>
<td class="w200x" align="center"><a href="howtosa2.php"> <img src="../images/select-image.png" alt="Ovládací prvky dvdisaster: Výběr souboru bitové kopie (vstupní pole a tlačítko)" class="noborder"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></a></td>
<td>  </td>
<td class="valignt"><b>Vyberte adresář a název souboru bitové kopie</b> pro kterou chcete vytvořit data pro opravu chyb. (Předpokládá se, že bitová kopie ve formátu ISO byla vytvořena pomocí jiných prostředků, např. pomocí vašeho programu pro vytváření CD/DVD/BD.)</td>
</tr>
</table>
<?php
}

if($way != 0) {
?>
<table>
<tr>
<td class="w200x" align="center"><a href="howtosa3.php"> <img src="../images/select-ecc.png" alt="Ovládací prvky dvdisaster: Výběr souboru pro opravu chyb (vstupní pole a tlačítko)" class="noborder"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></a></td>
<td>  </td>
<td class="valignt"><b>Vyberte adresář a název souboru</b> pro ukládaný soubor pro opravu chyb.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><a href="howtosa4.php"> <img src="images/create-icon.png" alt="Ovládací prvky dvdisaster: Vytvořit (tlačítko)" class="noborder"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></a></td>
<td>  </td>
<td class="valignt"><b>Vytvořte soubor pro opravu chyb</b> pomocí tlačítka &quot;Vytvořit&quot;.</td>
</tr>
</table>

<?php begin_howto_shot("Vytvoření souboru pro opravu chyb.","watch-create.png", "down-fork-arrow.png"); ?><b>Počkejte na dokončení procesu vytváření.</b> Délka tvorby závisí na velikosti bitové kopie a vybrané úrovni redundance. Např. vytvoření souboru pro opravu chyb za použití &quot;standardní&quot; redundance bude pro 4GB bitovou kopii DVD na běžném počítači trvat 5 minut.<?php end_howto_shot(); ?><table>
<tr>
<td class="w200x"align="center"><img src="../images/old-image.png" alt="Ikona: Starý soubor bitové kopie" class="nobordervalignm">     <img src="../images/ecc.png" alt="Ikona: Samostatný soubor s daty pro opravu chyb" class="nobordervalignm"></td>
<td>  </td>
<td class="valignt"><b>To je vše.</b> Nyní můžete smazat vytvořenou bitovou kopii. Musíte však uchovat soubor pro opravu chyb a chránit ho před poškozením. Pro rady týkající se <a href="howtos24.php">uchovávání souborů pro opravu chyb</a> si přečtěte následující stránku.</td>
</tr>
</table>

<p><a href="howtos24.php">Uchovávání souborů pro opravu chyb...</a> <?php
} /* end of if($way != 0) */
?> <!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>