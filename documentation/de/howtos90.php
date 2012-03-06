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
begin_page();
?>

<!--- Insert actual page content below --->

<h3>Abbild-Kompatibilität überprüfen</h3>

<b>Warum ISO-Abbilder verwendet werden.</b> Einige Funktionen von dvdisaster arbeiten mit 
ISO-Abbilddateien, die
auf der Festplatte abgelegt sind. Es ist aus technischen Gründen nicht
möglich, diese Funktionen direkt auf dem CD/DVD/BD-Laufwerk auszuführen,
da diese Laufwerke für die Zugriffsmuster von dvdisaster zu langsam sind
und dadurch schnell abgenutzt würden. Festplatten sind hingegen für diese
Art des Zugriffs gemacht und daher ausreichend schnell und standfest.<p>

<b>Die Überprüfung der Kompatibilität ist wichtig.</b> Wenn Sie mit 
dvdisaster arbeiten, können oder müssen Sie gegebenenfalls 
ISO-Abbilder verwenden, die von einer anderen Software erstellt worden sind.
ISO-Abbilder sind allerdings nur informell standardisiert. Typischerweise
produzieren zwar alle Programme die gleichen Abbilder, wenn als Dateiformat 
".iso" verwendet wird, aber es ist besser sicherzugehen daß
auch wirklich ein brauchbares ISO-Abbild erzeugt worden ist. 
Falls Sie nämlich ein anderes
Format mit dvdisaster verarbeiten, kann es sein daß die Fehlerkorrektur-Daten
unbrauchbar werden. Insbesondere können andere Formate 
wie <b>.nrg nicht verwendet</b> werden.
<p>

<b>Mögliche Szenarien.</b> In den folgenden beiden Situationen müssen ISO-Abbilder
zwischen dvdisaster und einer anderen Software ausgetauscht werden:<p>

<b>a) Fehlerkorrektur-Datei aus dem ISO-Abbild der Brennsoftware erstellen</b><p>

Sie erstellen mit Ihrer Brennsoftware ein ISO-Abbild, um daraus
einen Datenträger zu brennen und die Fehlerkorrektur-Datei zu erstellen.
Wenn Sie die Brennsoftware das erste Mal mit dvdisaster einsetzen,
vergewissern Sie sich, daß die <a href="howtos91.php">Abbilddatei unverändert
auf den Datenträger geschrieben wird</a>.<p>


<b>b) ISO-Abbilder mit Fehlerkorrektur-Daten erweitern</b><p>

dvdisaster führt eine "unsichtbare" Erweiterung der Abbilder mit
Fehlerkorrektur-Daten durch, damit andere Anwendungen durch diese
Daten nicht gestört werden.
Dies kann unter Umständen dazu führen, daß die 
Brennsoftware die Fehlerkorrektur-Daten nicht korrekt auf den Datenträger
schreibt. <a href="howtos92.php">Prüfen Sie daher nach</a>, ob Ihre
Brennsoftware die Fehlerkorrektur-Daten korrekt überträgt, wenn Sie sie
zum ersten Mal zusammen mit dvdisaster einsetzen.

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
