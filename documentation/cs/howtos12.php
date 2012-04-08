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

howto_headline("Kontrola poškození disků", "Postup", "images/scan-icon.png");
?>

<!-- Insert actual page content below -->Ujistěte se, že byl dvdisaster nastaven tak, jak je uvedeno v sekci <a href="howtos11.php">základní nastavení </a>,protože některé volby mohou nepříznivě ovlivnit výsledek kontroly. Poté postupujte podle následujícího návodu:<p><hr>

<table>
<tr>
<td class="w200x" align="center"><img src="../images/slot-in.png" alt="Ikona: Vložit disk do mechaniky"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů"></td>
<td>  </td>
<td class="valignt"><b>Vložte disk, který chcete zkontrolovat do mechaniky</b> která je přímo připojena k vašemu počítači. Nelze použít síťové mechaniky, softwarové mechaniky ani mechaniky virtuálních počítačů.</td>
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
<td class="valignt">V rozevírací nabídce dvdisaster <b>vyberte mechaniku, která obsahuje kontrolovaný disk</b>.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><a href="howtosa3.php"> <img src="../images/select-ecc.png" alt="Ovládací prvky dvdisaster: Výběr souboru pro opravu chyb (vstupní pole a tlačítko)" class="noborder"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></a></td>
<td>  </td>
<td class="valignt"><b>Vyberte soubor s daty pro opravu chyb,</b> pokud ho máte pro daný disk k dispozici. ECC data z RS02 rozšířeného disku jsou použita automaticky.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><a href="howtosa4.php"> <img src="images/scan-icon.png" alt="Ovládací prvky dvdisaster: Zkontrolovat (tlačítko)" class="noborder"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></a></td>
<td>  </td>
<td class="valignt">Spusťte kontrolu pomocí tlačítka <b> &quot;Zkontrolovat&quot;</b>.</td>
</tr>
</table>

<?php begin_howto_shot("Kontrola disku.","good-cd.png", ""); ?><b>Sledujte průběh kontroly.</b> Během kontroly neprovádějte na vašem počítači žádné další operace. Spouštění a práce s jinými programy, stejně jako třeba i jednoduché hýbání okny může ovlivnit výsledky kontroly.<?php end_howto_shot(); ?><p><hr><a href="howtos13.php">Interpretace výsledků...</a> <!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>