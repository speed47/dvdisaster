<?php
# dvdisaster: German homepage translation
# Copyright (C) 2004-2010 Carsten Gnörlich
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

<h3>Beispiele</h3>

Beispiele für <a href="howtos52.php?expand=1">gute Abbilder und
Fehlerkorrektur-Dateien</a> und <a href="howtos53.php?expand=1">gute mit
Fehlerkorrektur-Daten erweiterte Abbilder</a> haben Sie bereits auf den
vorherigen Seiten gesehen. Nachfolgend sind einige typische 
Beispiele für fehlerhafte Situationen dargestellt.<p>

<hr>

Die nächsten beiden Situationen sind typisch, wenn Sie Informationen
zu einem Abbild anzeigen, das noch nicht vollständig wiederhergestellt
worden ist:<p>

<?php begin_screen_shot("Abbild mit unlesbaren Sektoren, RS01","compare-bad-rs01.png"); ?>
<b>Abbild mit unlesbaren Sektoren und Fehlerkorrektur-Datei</b>.
Das hier gezeigte Abbild enthält 6245 unlesbare Sektoren; die
Fehlerkorrektur-Informationen liegen in Form einer
Fehlerkorrektur-Datei vor.
<p>
<?php end_screen_shot(); ?>

<?php begin_screen_shot("Abbild mit unlesbaren Sektoren, RS02","compare-bad-rs02.png"); ?>
<b>Mit Fehlerkorrektur-Daten erweitertes Abbild, das unlesbare Sektoren enthält</b>.
Das hier gezeigte Abbild enthält im hinteren Bereich unlesbare Sektoren.
Insbesondere der Ecc-Abschnitt ist betroffen, da die 
Fehlerkorrektur-Daten am Ende des Datenträgers liegen.
Das schwächt die Fehlerkorrektur allerdings nicht, da die 
Fehlerkorrektur-Leistung unabhängig von der Lage der Fehler ist: Es bedeutet
keinen Unterschied, ob 10000 Fehler am Anfang des Datenträgers oder am Ende
korrigiert werden müssen. Der RS02-Kodierer, der für die Erzeugung der erweiterten
Abbilder verwendet wird, kann außerdem eine Abschätzung liefern ob die
Fehlerkorrektur gelingen wird. Diese wird im unteren Bereich zu den 
Fehlerkorrektur-Daten angezeigt.
<?php end_screen_shot(); ?>

<hr>  

<?php begin_screen_shot("Abgebrochener Lesevorgang","compare-trunc-rs01.png"); ?>
<b>Abbild aus abgebrochenem Lesevorgang</b>.
Dieses Abbild ist kürzer als erwartet; dies passiert zum Beispiel
wenn der Lesevorgang vorzeitig abgebrochen wird.
<p>
<?php end_screen_shot(); ?>

<?php begin_screen_shot("Zu großes Abbild","compat-150-rs01.png"); ?>
<b>Abbild ist länger als erwartet</b>.
Dieses Abbild ist länger als erwartet; mögliche Fehlerursachen sind im
Abschnitt zur <a href="howtos90.php">Abbild-Kompatibilität</a> erklärt.
Unter Umständen gibt es Möglichkeiten zur Behebung des Problems
bei <a href="howtos91.php#err">Verwendung von Fehlerkorrektur-Dateien</a>
und bei <a href="howtos92.php#err">erweiterten Abbildern</a>.
<?php end_screen_shot(); ?>

<?php begin_screen_shot("Falsche Fehlerkorrektur-Datei","compare-mismatch-rs01.png"); ?>
<b>Falsche Fehlerkorrektur-Datei</b>.
Die verwendete Fehlerkorrektur-Datei wurde für ein anderes Abbild erzeugt.
Dadurch gibt es natürlich massenhaft Prüfsummenfehler, da der Inhalt
der Sektoren nicht stimmt. Der entscheidende Hinweis ist aber:<p>
Fingerabdruck: <font color="red">paßt nicht</font><p>
Daran sieht man daß die Fehlerkorrektur-Datei nicht zu dem Abbild gehört.
<?php end_screen_shot(); ?>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
