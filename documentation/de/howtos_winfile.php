<?php
# dvdisaster: German homepage translation
# Copyright (C) 2004-2010 Carsten Gnörlich
#
# UTF-8 trigger: äöüß 
?>

<a name="filechooser"></a>
<b>Hinweise zum Dateiauswahl-Dialog unter Windows und Mac OS X</b><p>

dvdisaster verwendet die <a href="http://www.gtk.org">GTK+</a>-Programmbibliothek,
die sicherstellt, daß es auf vielen verschiedenen Betriebssystemen läuft. 
Da die Bedienung des GTK-Dialogs zur Dateiauswahl unter Windows und Mac OS X ungewohnt ist,
finden Sie nachfolgend eine Übersicht zur Bedienung des Dialogs.
Ohne GTK+ wären die betriebssystemspezifischen Anpassungen für Windows und Mac OS X übrigens
zu aufwendig und es gäbe nur die GNU/Linux-Version von dvdisaster ;-)<p>

<?php begin_screen_shot("Dateiauswahl unter Windows","filebrowser.png"); ?>

<b>Vorhandene Dateien auswählen.</b>
Die <font color="#008000">grün</font> markierten Bereiche dienen zur Auswahl von Dateien. Wählen Sie zunächst
auf der linken Hälfte einen Ordner, dessen Inhalt anschließend in der rechten
Hälfte angezeigt und auswählbar wird.</b><p>

<b>Eine neue Datei auswählen.</b> 
Wählen Sie in dem <font color="#008000">grün</font> markierten Bereich links zunächst den Ordner, in dem Sie
eine Datei erstellen möchten. Geben Sie den Namen der neuen Datei in dem <font color="0000A0">blau</font> 
markierten Feld ein. Zur besseren Orientierung enthält die Überschrift des 
<font color="0000A0">blauen</font>
Feldes den Laufwerksbuchstaben und die gewählten Unterverzeichnisse, unter denen
die neue Datei angelegt wird.<p>

<b>Auf ein anderes Laufwerk wechseln.</b>
Die im System vorhandenen Laufwerksbuchstaben sind in dem <font color="#e0b000">gelb</font> markierten Bereich
aufgeführt. Wenn das Standardlaufwerk viele Unterverzeichnisse enthält, müssen
Sie in der linken Hälfte des Dialogs möglicherweise erst herunterblättern,
um die Laufwerksauswahl zu erreichen.<p>

<b>Eine Ordnerebene nach oben gehen.</b>
Klicken Sie auf den <font color="#a00000">rot</font> markierten Eintrag (zwei Punkte, ..) um eine
Verzeichnisebene nach oben zu gelangen. Alle vorherigen Verzeichnisebenen
sind über die rot markierte, mittig angeordnete Auswahl erreichbar.<br>
Achtung: Die rote markierte Auswahl dient <i>nicht</i> der Auswahl der Laufwerke;
dazu muß der gelb hervorgehobene Bereich (s.o.) verwendet werden.

<?php end_screen_shot(); ?>
