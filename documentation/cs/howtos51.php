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

howto_headline("Zjištění informací o obrazech a datech pro opravu chyb", "Zobrazení", "images/compare-icon.png");
?>

<!-- Insert actual page content below -->Pro tuto funkci neexistují žádná nastavení, ale je třeba zadat bitovou kopii a případně i k ní náležející <a href="howtos20.php">soubor pro opravu chyb</a>.<hr>

<table>
<tr>
<td class="w200x" align="center"><a href="howtosa2.php"> <img src="../images/select-image.png" alt="Ovládací prvky dvdisaster: Výběr souboru bitové kopie (vstupní pole a tlačítko)" class="noborder"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></a></td>
<td>  </td>
<td class="valignt"><b>Zadejte název souboru bitové kopie</b>, o němž chcete získat informace. Bitová kopie již musí být uložena na pevném disku, v opačném případě ji vytvořte z disku pomocí funkce &quot;Načíst&quot;.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><a href="howtosa3.php"> <img src="../images/select-ecc.png" alt="Ovládací prvky dvdisaster: Výběr souboru pro opravu chyb (vstupní pole a tlačítko)" class="noborder"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></a></td>
<td>  </td>
<td class="valignt"><b>Zadejte název souboru pro opravu chyb</b>, který patří k výše zvolené bitové kopii. Pokud byla bitová kopie <a href="howtos30.php">rozšířena o data pro opravu chyb</a>, ponechte tuto položku prázdnou.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><a href="howtosa4.php"> <img src="images/compare-icon.png" alt="Ovládací prvky dvdisaster: Ověřit (tlačítko)" class="noborder"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></a></td>
<td>  </td>
<td class="valignt"><b>Spusťte vyhodnocení</b> pomocí tlačítka &quot;Ověřit&quot;.</td>
</tr>
</table>

<?php begin_howto_shot("zobrazení informací.","compat-okay-rs01.png", ""); ?><b>Sledujte průběh ověření.</b> Pro zobrazení všech informací musí být přečtena celá bitová kopie i data pro opravu chyb.<?php end_howto_shot(); ?><hr><a name="examine">Další informace o interpretaci výsledků:</a><p><ul>
<li><a href="howtos52.php">Vysvětlení výsledků pro soubory pro opravu chyb</a><p></li>
<li><a href="howtos53.php">Vysvětlení výsledků pro rozšířené bitové kopie</a><p></li>
<li><a href="howtos59.php">Příklady</a><p></li>
</ul>


<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
