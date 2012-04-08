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

<h3 class="top">Systemanforderungen</h3>

<ul>
 <li>x86-, PowerPC- oder Sparc-Prozessor;</li>
 <li>ein aktuelles CD-/DVD-/BD-Laufwerk mit ATAPI-, SATA- oder SCSI-Schnittstelle;</li>
 <li>ausreichend Festplattenplatz zum Abspeichern eines ISO-Abbildes der bearbeiteten Datenträger.</li>
</ul>

<h3 class="top">Unterstützte Betriebssysteme</h3>

Die folgende Tabelle enthält Informationen zu den unterstützten
Betriebssystemen. Die angegebenen Versionen wurde zum Entwickeln
und Testen der momentanen dvdisaster-Version verwendet. 
Es ist wahrscheinlich, daß etwas ältere oder neuere Betriebssystem-Versionen
ebenfalls verwendet werden können.<p>

Das dvdisaster-Projekt empfiehlt GNU/Linux.<p>

<table border="1">
<tr>
<td>Betriebssystem</td>
<td>Version</td>
<td>32bit-Version</td>
<td>64bit-Version</td>
</tr>
<tr>
<td>GNU/Linux</td>
<td>Debian Squeeze (6.0.4)<br>Kernel 2.6.32</td>
<td align="center">ja</td>
<td align="center">ja</td>
</tr>
<tr>
<td>FreeBSD<sup>1)</sup></td>
<td>9.0</td>
<td align="center">ja</td>
<td align="center">ja</td>
</tr>
<tr>
<td>NetBSD</td>
<td>5.1.2</td>
<td align="center">ja</td>
<td align="center">ja</td>
</tr>
<tr>
<td>Mac OS X</td>
<td>10.6 (Snow Leopard)</td>
<td align="center">ja</td>
<td align="center">nein<sup>2)</sup></td>
</tr>
<tr>
<td>Windows<sup>4)</sup></td>
<td>Windows 2000 SP4<sup>3)</sup></td>
<td align="center">ja</td>
<td align="center">nein<sup>2)</sup></td>
</tr>
</table><p>

<sup>1)</sup>FreeBSD: für ATAPI-Laufwerke muß das Kernelmodul <i>atapicam</i> geladen sein -- siehe das INSTALL-Dokument<br>
<sup>2)</sup>64bit-Unterstützung nicht geplant.<br>
<sup>3)</sup>Nutzer haben positive Rückmeldungen über den Einsatz mit
späteren Versionen bis einschließlich Windows 7 gegeben.<br>
<sup>4)</sup>Die Unterstützung von Mehrkernprozessoren hängt von der jeweiligen
Windows-"Edition" ab. Bei einigen ergeben zusätzliche Prozessorkerne mit
dvdisaster keine Verbesserung der Gechwindigkeit.

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
