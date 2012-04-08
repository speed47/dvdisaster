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
begin_page();
?>

<!-- Insert actual page content below -->

<h3 class="top">Ein paar Hintergründe zu Lesefehlern</h3>

Optische Medien verfügen über einen eigenen Fehlerkorrektur-Kode, der kleinere
Material- und Brennfehler ausgleicht und so die gespeicherten Daten
schützt. Wenn der Brenner und die Rohlinge kompatibel und 
von hoher Qualität sind, ist die eingebaute Fehlerkorrektur 
direkt nach dem Brennen nur schwach ausgelastet. Sie verfügt dann
über genügend Kapazität, um die während der Benutzung des Datenträgers
auftretenden Verschleiß- und Alterungserscheinungen für viele Jahre
auszugleichen.<p>

Erst wenn die Reserven der eingebauten Fehlerkorrektur erschöpft sind,
entstehen Lesefehler, die Sie in dvdisaster mit 
der <a href="howtos10.php">"Prüfen"</a>-Funktion feststellen können.
Dabei ist die folgende Unterscheidung wichtig:<p>


<b>Lesefehler direkt nach dem Brennen.</b> <br>Wenn unlesbare Sektoren
direkt nach dem Brennen auftreten, ist das ein Hinweis auf

<ul>
<li>Produktionsfehler bei den Rohlingen, oder</li>
<li>Rohlinge, die nicht mit dem Brenner kompatibel sind</li>
</ul>

In diesem Fall hilft nur ein Entsorgen der defekten Rohlinge und ein
erneutes Brennen auf einwandfreie Ware, gegebenenfalls verbunden mit
einem Herstellerwechsel.<p>

Der Versuch, derartige Fehlbrände mit Hilfe einer Fehlerkorrekturdatei
am Leben zu erhalten, endet hingegen ziemlich sicher mit einem Datenverlust.<p>

<b>Lesefehler nach einigen Monaten/Jahren.</b> Die eingebaute Fehlerkorrektur des
Datenträgers wird mit zunehmender Lebensdauer immer stärker belastet, 
bis schließlich Lesefehler entstehen.
Dies hat sowohl mechanische Ursachen (Kratzer, Verziehen des Materials) als
auch chemische Hintergründe (Zerfall des Farbstoffes und/oder der Spiegelschicht).<p>

Typischerweise treten diese Effekte auf, während der Datenträger für einige 
Monate gelagert wird, und es ist danach auch mit den unten beschriebenen Tips
nicht mehr möglich, den Datenträger wieder komplett einzulesen. <p>

Deshalb ist es wichtig, rechtzeitig mit dvdisaster die zugehörigen
<a href="howtos21.php">Fehlerkorrektur-Daten</a> zu erzeugen, weil
dadurch innerhalb <a href="qa31.php">bestimmter Grenzen</a> 
der Inhalt von Sektoren berechnet (= wiederhergestellt) werden kann,
die von keinem Laufwerk mehr gelesen werden können.<p>

Dabei braucht man den Datenträger typischerweise nicht bis auf den letzten
lesbaren Sektor "auszuquetschen": 
Das <a href="qa35.php">angepaßte Leseverfahren</a>
von dvdisaster überprüft beim Lesen ständig, ob genügend Daten für
die Fehlerkorrektur zur Verfügung stehen. Sobald dies der Fall ist,
wird der Lesevorgang beendet und die bis dahin noch nicht gelesenen
Sektoren werden aus den Fehlerkorrektur-Daten wiederhergestellt.<p>

<a name="reading-tips"><b>Einige Tips zum effektiven Auslesen beschädigter Datenträger</b></a><p>

Die "Ausbeute" beim Lesen beschädigter Datenträger hängt von mehreren Umständen ab:

<ul>
<li><b>Nicht alle Laufwerke sind gleich gebaut.</b><br>
Verschiedene Laufwerke haben auch verschieden gute Lesefähigkeiten. 
Nutzen Sie die Möglichkeit von dvdisaster, ein Abbild mit mehreren Lesevorgängen
zu vervollständigen, und setzen Sie dabei verschiedene Laufwerke ein. 
Übertragen Sie die Abbild-Datei mit Hilfe eines Netzwerks oder mit RW-Datenträgern,
um Laufwerke in verschiedenen Rechnern einsetzen zu können.<p></li>
<li><b>Auswerfen und wieder einlegen.</b><br>
Ab und zu hilft es, den Datenträger auszuwerfen, 
ihn um ein Viertel zu drehen und dann einen neuen Leseversuch zu starten.<p></li>
<li><b>Kalte Laufwerke lesen besser.</b><br>
Einige Laufwerke haben im kalten Zustand bessere Leseeigenschaften.
Schalten Sie den Rechner über Nacht aus und versuchen Sie es am nächsten Morgen noch mal.<p>
"Kalt" heißt übrigens Raumtemperatur - Ein Aufenthalt im Kühlschrank ist für 
Hardware und Datenträger ziemlich ungesund.<p></li>
</ul>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
