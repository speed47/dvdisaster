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

<h3 class="top">Dialogy a tlačítka</h3>Tato sekce popisuje běžně používané dialogy a tlačítka:<pre> </pre>

<table width="100%">
<tr>
<td align="center"><a href="howtosa1.php"><img src="../images/good-cd.png" alt="Ovládací prvky dvdisaster: Výběr mechaniky (rozevírací nabídka)" class="noborder"></a></td>
<td><a href="howtosa1.php">Nabídka pro výběr mechaniky</a>.</td>
</tr>
<tr><td> </td><td></td></tr>
<tr>
<td align="center"><a href="howtosa2.php"><img src="../images/good-image.png" alt="Ovládací prvky dvdisaster: Výběr souboru bitové kopie (tlačítko)" class="noborder"></a></td>
<td><a href="howtosa2.php">Okno pro výběr bitové kopie</a>.</td>
</tr>
<tr><td> </td><td></td></tr>
<tr>
<td align="center"><a href="howtosa3.php"><img src="../images/ecc.png" alt="Ovládací prvky dvdisaster: Výběr souboru pro opravu chyb (tlačítko)" class="noborder"></a></td>
<td><a href="howtosa3.php">Okno pro výběr souboru pro opravu chyb</a>.</td>
</tr>
<tr><td> </td><td></td></tr>
<tr>
<td align="center"><a href="howtosa4.php"> <img src="images/read-icon.png" alt="Ovládací prvky dvdisaster: Načíst (tlačítko)" class="noborder"> <img src="images/create-icon.png" alt="Ovládací prvky dvdisaster: Vytvořit (tlačítko)" class="noborder"><br> <img src="images/scan-icon.png" alt="Ovládací prvky dvdisaster: Zkontrolovat (tlačítko)" class="noborder"> <img src="images/fix-icon.png" alt="Ovládací prvky dvdisaster: Opravit (tlačítko)" class="noborder"><br> <img src="images/compare-icon.png" alt="Ovládací prvky dvdisaster: Ověřit (tlačítko)" class="noborder"> <img src="images/stop-icon.png" alt="Ovládací prvky dvdisaster: Zastavit (tlačítko)" class="noborder"> </a></td>
<td><a href="howtosa4.php">Tlačítka pro spuštění akcí</a>.</td>
</tr>
</table>



<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
