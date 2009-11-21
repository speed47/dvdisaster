<?php
# dvdisaster: German homepage translation
# Copyright (C) 2004-2009 Carsten Gnörlich
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

<!--- Insert actual page content below --->

<h3>Laufwerk auswählen</h3>

<?php begin_screen_shot("Laufwerk auswählen","dialog-drive-full.png"); ?>
Das Auswahlfeld für die Laufwerke befindet sich oben links in der
Menüleiste von dvdisaster (siehe grüne Markierung).
Klicken Sie mit der Maus auf das Feld, um die Auswahlliste auszuklappen.
Wählen Sie dann den Eintrag mit dem Laufwerk, 
in das Sie Ihren Datenträger gelegt haben.<p>

Um das Auffinden des Laufwerks in der Liste zu erleichtern
sind dort zwei Bezeichnungen angegeben:
<ul>
<li>Die Gerätebezeichnung, aus der typischerweise der Hersteller und der Laufwerkstyp
hervorgehen. dvdisaster übernimmt an dieser Stelle die Bezeichnung, 
die der Hersteller in das Laufwerk einprogrammiert hat. 
Manchmal ist dieser Text nicht sehr aussagekräftig.<p></li>
<li>Die Kennung, unter der das Betriebssystem das Laufwerk verwaltet (z.B. /dev/hda unter GNU/Linux oder F: unter Windows)</li>
</ul>
<?php end_screen_shot(); ?>


<p>
<b>Beispiele:</b>
<table width="100%">
<tr>
<td width="50%" align="center"><img src="images/select-drive-linux.png"><br>
Ausgeklappte Auswahl unter GNU/Linux</td>
<td width="50%" align="center"><img src="images/select-drive-win.png"><br>
Ausgeklappte Auswahl unter Windows</td>
</tr>
</table><p>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
