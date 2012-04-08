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

<h3 class="top">Spuštění akcí</h3>

<?php begin_screen_shot("Spuštění akcí","action-buttons.png"); ?>Pro spuštění některé z akcí programu dvdisaster klikněte na některé ze zeleně označených tlačítek:<p><table>
<tr>
<td class="valignt"><img src="images/read-icon.png" alt="Ovládací prvky dvdisaster: Načíst (tlačítko)">  </td>
<td><b>Načtení obsahu disku do bitové kopie</b> pro:<ul>
<li>načtení <a href="howtos42.php#a">poškozeného disku</a> pro následnou opravu.<li>načtení <a href="howtos23.php?way=1&expand=0">nepoškozeného disku</a> pro vytvoření souboru pro opravu chyb.</ul></td>
</tr>

<tr>
<td class="valignt"><img src="images/create-icon.png" alt="Ovládací prvky dvdisaster: Vytvořit (tlačítko)">  </td>
<td><b><a href="howtos20.php">Vytvoření souboru pro opravu chyb</a></b><br>(možné pouze z nepoškozeného disku!)</td>
</tr>

<tr>
<td class="valignt"><img src="images/scan-icon.png" alt="Ovládací prvky dvdisaster: Zkontrolovat (tlačítko)">  </td>
<td><b><a href="howtos10.php">Kontrola čitelnosti disku.</a></b></td>
</tr>

<tr>
<td class="valignt"><img src="images/fix-icon.png" alt="Ovládací prvky dvdisaster: Opravit (tlačítko)">  </td>
<td><b><a href="howtos40.php">Oprava bitové kopie poškozeného disku</a></b><br>za předpokladu že jsou k dispozici <a href="howtos20.php">data pro opravu chyb</a>.</td>
</tr>

<tr>
<td class="valignt"><img src="images/compare-icon.png" alt="Ovládací prvky dvdisaster: Ověřit (tlačítko)">  </td>
<td>Zobrazení <a href="howtos50.php">informací o bitových kopiích a datech pro opravu chyb</a>.</td>
</tr>
</table><p><b>Ostatní tlačítka související s výše uvedenými akcemi:</b><table>
<tr>
<td class="valignt"><img src="images/log-icon.png" alt="Ovládací prvky dvdisaster: Zobrazit záznam (tlačítko)">  </td>
<td><b>Zobrazení souboru záznamu právě probíhající akce</b> (označeno žlutě).<br>Viz také: <a href="feedback.php#log">Vytvoření souboru se záznamem</a>.</td>
</tr>

<tr>
<td class="valignt"><img src="images/stop-icon.png" alt="Ovládací prvky dvdisaster: Zastavit (tlačítko)">  </td>
<td><b>Zrušení probíhající akce</b> (označeno červeně).<br>U některých akcí může zrušení trvat delší dobu, zvláště pokud tlačítko stlačíte ve chvíli kdy je čten poškozený sektor.</td>
</tr>
</table>

<?php end_screen_shot(); ?>


<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
