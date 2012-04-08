<?php
# dvdisaster: German homepage translation
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

<h3 class="top">Abbild-Datei auswählen</h3>

Die Abbild-Datei enthält den Inhalt des Datenträgers, ergänzt um Informationen
ob die betreffenden Stellen lesbar oder unlesbar waren. dvdisaster verwendet
Abbild-Dateien, weil diese auf der Festplatte liegen und damit bestimmte 
Zugriffsmuster wesentlich schneller ablaufen. CD/DVD-Laufwerke würden durch diese
Zugriffsmuster ausgebremst und in kurzer Zeit abgenutzt werden (dies gilt nicht 
für das Erstellen der Abbild-Dateien!).
Die voreingestellte 
Dateinamen-Erweiterung für Abbilder ist ".iso".<p>

<?php begin_screen_shot("Abbild-Datei auswählen","dialog-iso-full.png"); ?>
Sie können die Abbild-Datei auf zwei Arten auswählen:
<ul>
<li>im normalen <a href="#filechooser">Dateiauswahl-Dialog</a> (grün markierter Knopf), oder</li>
<li>durch Direkteingabe in das Textfeld (blau markiertes Textfeld).</li>
</ul><p>
Die Direkteingabe ist hilfreich, 
wenn Sie mehrere Abbild-Dateien nacheinander im gleichen
Unterverzeichnis erzeugen. Ändern Sie dazu einfach den Namen in dem Textfeld.<p>
<?php end_screen_shot(); ?>

<?php require("howtos_winfile.php"); ?>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
