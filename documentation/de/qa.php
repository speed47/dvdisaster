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

<h3><a name="top">Allgemeine Fragen und Antworten</a></h3>

<a href="#pronounce">1.1 Wie spricht man "dvdisaster" aus?</a><p>
<a href="#pipo">1.2 Was sind Qualitäts-Analysen und warum werden nicht mehr unterstützt?</a><p>
<a href="#compat">1.3 Ist dvdisaster mit nachfolgenden Versionen kompatibel?</a>

<hr><p>

<b><a name="pronounce">1.1 Wie spricht man "dvdisaster" aus?</a></b><p>
Da der Wortstamm <i>disaster</i> aus dem Englischen kommt, 
spricht man es etwa wie "diwidisaster" aus. 
<div align=right><a href="#top">&uarr;</a></div>


<b><a name="pipo">1.2 Was sind Qualitäts-Analysen und warum werden nicht mehr unterstützt?</a></b><p>
Optische Datenträger enthalten einen Fehlerkorrektur-Mechanismus, der nach einem
ähnlichen Prinzip wie dvdisaster arbeitet.
Einige Laufwerke können beim Lesen eines Datenträgers 
die Anzahl der korrigierten Fehler zurückmelden. Daraus ergibt sich eine grobe
Abschätzung der Brenn- und Datenträgerqualität.<p>

Weil dvdisaster freie Software ist, kann es nur Programmkode und Informationen
verwenden, die frei veröffentlicht werden dürfen. Dies ist momentan nur der Fall
für <a href="howtos10.php">C2-Analysen</a> von CD-Datenträgern, denn diese sind
ein offizieller Standard und es gibt frei verfügbare Dokumentation dazu.<p>

Andererseits sind DVD-Qualitäts-Analysen ("PI/PO-Analysen") nicht standardisiert.
Sie werden von einigen Laufwerken mit undokumentierten Programmierschnittstellen
unterstützt, aber die dazugehörigen Beschreibungen scheinen nicht für freie
Software verfügbar zu sein. Daher müssen wir abwarten bis die Hersteller einsehen,
daß sich ihre Produkte umso besser verkaufen, je mehr freie Software für ihre
Laufwerke verfügbar ist. <p>

<div align=right><a href="#top">&uarr;</a></div><p>

<b><a name="compat">1.3 Ist dvdisaster mit nachfolgenden Versionen kompatibel?</a></b><p>
Ja, denn dvdisaster ist für eine Datenarchivierung über viele Jahre vorgesehen. 
Sie können beim Umstieg auf eine neuere Version von dvdisaster die Abbild- und
Fehlerkorrekturdateien von Vorgängerversionen weiter verwenden und brauchen
diese <i>nicht</i> neu zu erzeugen.<p>
<div align=right><a href="#top">&uarr;</a></div><p>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
