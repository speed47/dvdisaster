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

howto_headline("Datenträger-Abbild rekonstruieren", "fortgeschrittene Einstellungen", "images/create-icon.png");
?>

Normalerweise wird das erste <a href="howtos42.php">Einlesen des defekten Datenträgers</a> 
bereits genügend Daten für die Fehlerkorrektur liefern. Wenn dies nicht der Fall ist,
gehen Sie wie folgt vor:<p>

<?php begin_screen_shot("Abschätzen der Erfolgsaussichten","adaptive-failure.png"); ?>
<b>Abschätzen der Erfolgsaussichten</b><p>
Betrachten Sie die Ausgabe des Lesevorgangs. 
Unter der Rubrik "Bearbeitete Sektoren" finden Sie die Angabe, 
welcher Prozentteil der Sektoren lesbar waren und wieviele benötigt werden.
Anhand der Differenz der beiden Werte (im Beispiel 85.6% - 76.5% = 9.1%) können Sie die
Erfolgsaussichten abschätzen, noch genügend Sektoren für eine erfolgreiche 
Wiederherstellung zusammenzubekommen:<p>
<?php end_screen_shot(); ?>

<table cellspacing="0" cellpadding="10px">
<tr bgcolor="#c0ffc0">
<td width="10%" align="center" valign="top">&lt; 5%</td>
<td>Die Aussichten sind gut, daß Sie mit mehreren Leseversuchen genügend
Daten erhalten.
</td></tr>
<tr bgcolor="#ffffc0">
<td width="10%" align="center" valign="top">5%-10%</td>
<td> Wenn Sie mehrere Laufwerke haben sind die Aussichten gut, daß Sie
bei entsprechender Ausdauer noch genügend Daten zusammenbekommen.
</td></tr>
<tr bgcolor="#ffe0c0">
<td width="10%" align="center" valign="top">10%-20%</td>
<td> Das wird knapp. Wenn Sie mit den nächsten 2 bis 3 Leseversuchen nicht deutlich 
unter 10% kommen, ist der Inhalt des Datenträger vermutlich nicht mehr zu retten.
</td></tr>
<tr bgcolor="#ffc0c0">
<td width="10%" align="center" valign="top">&gt; 20%</td>
<td>Der Datenverlust ist sehr wahrscheinlich zu groß,
um den Inhalt noch wiederherstellen zu können.
Verwenden Sie in Zukunft höhere Redundanzen für die Fehlerkorrektur-Daten
und  testen Sie Ihre Datenträger in kürzeren Abständen.</td></tr>
</table><p>

Probieren Sie die nachfolgenden Empfehlungen, um weitere Leseversuche durchzuführen.
Führen Sie zunächst zu jeder Empfehlung einen vollständigen Lesevorgang durch, damit
Sie abschätzen können wie die jeweiligen Einstellungen wirken. Später können Sie
sich aus diesen Einstellungen dann selbst eine optimale Mischung zusammenstellen.<p>

<hr>

<?php begin_screen_shot("Führen Sie einen erneuten Leseversuch durch","fix-prefs-read-attempts1.png"); ?>
<b>Führen Sie einen erneuten Leseversuch durch</b><p>
Stellen Sie einen kleineren Wert zum Beenden des Lesevorgangs ein 
(empfohlen: 32 für BD, 16 für DVD und 0 für CD). Lassen Sie den Lesevorgang damit erneut 
durchlaufen. Solange jeder Durchlauf noch eine gute Ausbeute an neuen Sektoren
erbringt, können Sie diesen Schritt auch mehrfach wiederholen.<p>
<b>Tip:</b> Lassen Sie das Laufwerk zwischen den Durchläufen abkühlen. Werfen Sie den
Datenträger vor jedem Durchlauf aus und ziehen Sie ihn wieder ein; manchmal 
liegt der Datenträger danach in einer besseren Position 
und die Anzahl der lesbaren Sektoren verbessert sich.
<p>
<?php end_screen_shot(); ?>

<hr>

<b>Vervollständigen Sie das Abbild mit mehreren Laufwerken</b><p>
Unternehmen Sie weitere Leseversuche mit anderen Laufwerken. Übertragen
Sie das Abbild bei Bedarf auf andere Computer, um es mit den dort eingebauten
Laufwerken weiter zu vervollständigen.</b><p>

<hr>

<?php begin_screen_shot("Setzen Sie die Anzahl der Leseversuche hoch","fix-prefs-read-attempts2.png"); ?>
<b>Setzen Sie die Anzahl der Leseversuche hoch</b><p>
<i>Für alle Datenträgertypen (CD, DVD, BD):</i><p>
Stellen Sie für die Anzahl der Leseversuche einen Minimalwert von 5 und einen Maximalwert von 9 
ein (grüne Markierungen).<p>
<i>Nur bei CD-Datenträgern:</i><p>
Bei CD-Datenträgern ist es mit einigen Laufwerken möglich, beschädigte Sektoren teilweise zu lesen.
Wählen Sie die Funktion "Unvollständige Roh-Sektoren ... aufbewahren" aus und geben Sie ein
Verzeichnis an, in dem die Bruchstücke von beschädigten Sektoren aufbewahrt werden sollen
(gelbe Markierungen). Aus
den so gesammelten Informationen kann dvdisaster später möglicherweise einige Sektoren wieder
zusammensetzen.
<?php end_screen_shot(); ?>


<?php begin_screen_shot("Mehrfache Leseversuche beobachten","fix-reread-dvd.png"); ?>

<i>Wirkung der mehrfachen Leseversuche prüfen (CD, DVD, BD):</i><br>
Nicht bei allen Laufwerken ergibt sich durch diese Einstellungen eine Verbesserung.
Ausgaben der Art "Sektor ...: Versuch x: erfolgreich" (Fußzeile unten,
gelbe Markierung) bedeuten, daß Sektoren durch mehrfache Leseversuche gelesen werden konnten.
Falls Sie solche Ausgaben nie bekommen, lohnt es sich bei dem betreffenden Laufwerk nicht,
die Anzahl der Leseversuche zu erhöhen.  
<?php end_screen_shot(); ?>

<a name="21h"></a>
<i>Teilweises Lesen von beschädigten CD-Sektoren prüfen:</i><br>
Schauen Sie nach dem Lesevorgang in den eben ausgewählten Ordner 
(im Beispiel /var/tmp/raw). Wenn darin keine Raw-Dateien angelegt 
worden sind, beherrscht das Laufwerk möglicherweise den benötigten
Lesemodus nicht. Haben Sie hingegen mehrere Laufwerke, die Raw-Dateien
erzeugen, dann lassen Sie alle Laufwerke im selben Verzeichnis auf
den  Raw-Dateien arbeiten, da sich Sektorenbruchstücke von 
verschiedenen Laufwerken besonders gut ergänzen. 
<p>

<?php begin_screen_shot("Anderes RAW-Verfahren verwenden","fix-prefs-drive2.png"); ?>
<i>Anderes RAW-Verfahren für CD einsetzen:</i><br>
Bei einigen Laufwerken bringt das voreingestellte Leseverfahren "20h" keinen Erfolg.
Probieren Sie einen weiteren Lesedurchlauf mit der Einstellung "21h" (siehe
Bildschirmfoto) und schauen Sie erneut nach, ob Raw-Dateien entstehen.
<?php end_screen_shot(); ?>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
