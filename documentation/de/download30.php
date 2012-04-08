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

<h3 class="top">Installation der Quellkode-Pakete</h3>

dvdisaster verwendet den üblichen
<tt>./configure; make; make install</tt>-Mechanismus für die Installation
aus dem Quellkode. In dem Quellkode-Paket ist eine Datei
<tt>INSTALL</tt> mit weiteren Information enthalten.
<pre> </pre>

<a name="mac"></a>
<h3>Installieren der Binärversion für Mac OS X</h3>

Das ZIP-Archiv enthält ein "application bundle" für Mac OS X 10.5
und x86-Prozessoren.
Packen Sie das Archiv an einer beliebigen Stelle aus und klicken Sie
dann auf "dvdisaster.app" um das Programm zu starten.
dvdisaster ist auch unter Mac OS X 10.4 und mit 
PowerPC-Prozessoren lauffähig; für diese Fälle müssen Sie es allerdings
selbst aus dem Quellkode bauen.<p>

<i>Bitte beachten Sie daß die Benutzeroberfläche an einigen Stellen noch
hakt.</i> Die Benutzeroberflächen-Bibliothek GTK+ befindet sich für Mac
OS X noch in einem frühen Entwicklungsstadium. Die 
Benutzeroberfläche kann kurzzeitig ruckeln oder einfrieren und es gibt
unter Umständen Graphikfehler in der Ausgabe. Sie können diese Effekte
minimieren indem Sie möglichst nicht mit dem dvdisaster-Fenster spielen
während darin eine Aktion abläuft. Vermeiden Sie es inbesondere die
Fenstergröße zu verändern oder das Fenster in das Dock zu schicken.<p>

Im Verlauf des 0.73er Entwicklungszweiges werden möglicherweise Workarounds
gefunden um diese Effekte zu minimieren; ansonsten heißt es abwarten
bis verbesserte Versionen von GTK+ erscheinen.<p>

Die Entwicklung einer GTK-losen dvdisaster-Version ist nicht geplant.
Das Programm müßte für Quartz komplett neu geschrieben werden.
Für die Windows-Version gilt übrigens sinngemäß das Gleiche ;-)

<pre> </pre>

<a name="win"></a>
<h3>Installieren der Binärversion für Windows</h3>

Zum Installieren der Windows-Version führen Sie bitte das
Installations-Programm (z.B. <?php echo ${pkgname}?>-setup.exe) 
aus. Es enthält einen Dialog zum Einrichten von dvdisaster.<p>

<b>Warnung:</b> Sie können dvdisaster nicht installieren, indem Sie
das setup.exe-Programm von Hand auspacken oder indem Sie Teile von
dvdisaster aus einer bereits erfolgten Installation kopieren.
Wird dvdisaster nicht durch das setup.exe-Programm installiert,
so erscheinen seltsame Fehler(-meldungen), 
die nicht immer nachvollziehbar mit einer unvollständigen Installation
zusammenhängen.

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
