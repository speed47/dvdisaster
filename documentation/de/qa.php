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

<h3 class="top"><a name="top">Allgemeine Fragen und Antworten</a></h3>

<a href="#pronounce">1.1 Wie spricht man "dvdisaster" aus?</a><p>
<a href="#pipo">1.2 Was sind Qualitäts-Analysen und warum werden nicht mehr unterstützt?</a><p>
<a href="#compat">1.3 Ist dvdisaster mit nachfolgenden Versionen kompatibel?</a><p>
<a href="#eccpos">1.4 Bei erweiterten Abbildern liegen die Fehlerkorrektur-Daten am Ende des Datenträgers. Ist das eine schlechte Wahl?</a><p>
<a href="#recovery">1.5 Was sind die Unterschiede zwischen Fehlerkorrektur auf Datei- und Abbild-Ebene?</a><p>
<hr><p>

<b><a name="pronounce">1.1 Wie spricht man "dvdisaster" aus?</a></b><p>
Da der Wortstamm <i>disaster</i> aus dem Englischen kommt, 
spricht man es etwa wie "diwidisaster" aus. 
<div class="talignr"><a href="#top">&uarr;</a></div>


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

<div class="talignr"><a href="#top">&uarr;</a></div><p>

<b><a name="compat">1.3 Ist dvdisaster mit nachfolgenden Versionen kompatibel?</a></b><p>
Ja, denn dvdisaster ist für eine Datenarchivierung über viele Jahre vorgesehen. 
Sie können beim Umstieg auf eine neuere Version von dvdisaster die Abbild- und
Fehlerkorrekturdateien von Vorgängerversionen weiter verwenden und brauchen
diese <i>nicht</i> neu zu erzeugen.<p>
<div class="talignr"><a href="#top">&uarr;</a></div><p>

<b><a name="eccpos">1.4 Bei erweiterten Abbildern liegen die Fehlerkorrektur-Daten am Ende des Datenträgers. Ist das eine schlechte Wahl?</a></b><p>
Nein. Zunächst eine kleine Begriffsbestimmung:
Wenn wir 80 Bytes Nutzerdaten mit 20 Bytes Fehlerkorrektur-Daten erweitern,
dann erhalten wir einen "Ecc-Block", der aus 100 Bytes besteht.
Nun betrachten Sie die folgenden Überlegungen zu dem Ecc-Block:

<ol>
<li>Es ist egal, wo die Fehlerkorrektur-Daten innerhalb des Ecc-Blocks liegen.
<p>
Der RS-Dekoder unterscheidet nicht zwischen Nutzerdaten und 
Fehlerkorrektur-Daten. Für ihn besteht der Ecc-Block aus 100 Bytes,
von denen er eine beliebige Menge aus 20 Bytes wiederherstellen kann.
Das können die ersten oder letzen 20 Bytes sein, aber auch
jede Kombination von 20 Bytes dazwischen kann wiederhergestellt werden,
solange die restlichen 80 Bytes
noch in Ordnung sind. Daraus folgt daß die Position der Fehlerkorrekur-Daten
innerhalb des Ecc-Blocks egal ist. Die Leistungsfähigkeit der 
Fehlerkorrektur wird nicht davon beeinflußt, ob die Fehlerkorrektur-Daten
am Anfang oder Ende der Nutzerdaten eingefügt werden oder ob sie gar mit
den Nutzerdaten vermischt sind.</li>

<li>Eine gleichförmige Verteilung des Ecc-Blocks kompensiert den Einfluß schlechter Datenträger-Bereiche.<p>
Optische Datenträger haben höhere Ausfallwahrscheinlichkeiten in den
äußeren Bereichen; aus technischen Gründen müssen aber hier die
Fehlerkorrektur-Daten gespeichert werden. Dieser Effekt wird jedoch
dadurch kompensiert, daß der Inhalt der Ecc-Blöcke gleichmäßig über
den Datenträger verteilt wird. Nehmen wir an daß der Datenträger zu 80%
mit Nutzerdaten gefüllt ist. Dadurch verbleiben 20% am äußeren Rand für
Fehlerkorrektur-Daten. Betrachten wir jetzt wieder den Ecc-Block aus
100 Bytes. Um ihn zusammenzubauen nehmen wir 80 Bytes aus dem Bereich der
Nutzerdaten und 20 zusätzliche Bytepostionen aus dem Fehlerkorrektur-Bereich.
Selbst unter diesen Einschränkungen können die 100 Bytes des Ecc-Blocks
gleichmäßig über den Datenträger verteilt werden, von innen nach außen
und mit einem jeweils maximal großen Abstand zu ihren Nachbarn.
Zusammen mit Punkt (1) werden so die Einflüsse schlechter Datenträger-Bereiche
kompensiert: Aus Symmetriegründen existiert für jedes Fehlerkorrektur-Byte
im (schlechten) äußeren Bereich ein Nutzerdaten-Byte im (guten) inneren
Bereich des Datenträgers. <p>
(Wenn Sie die Argumentation noch nicht nachvollziehen können, stellen Sie
sich vor, die Fehlerkorrektur-Daten in dem inneren Bereich des Datenträgers
zu speichern und die Nutzerdaten im äußeren Bereich. Überdenken Sie
Punkt (1) erneut um zu sehen daß sich dadurch für die Fehlerkorrektur
nichts verbessert.)
</li>
</ol>
<div class="talignr"><a href="#top">&uarr;</a></div><p>

<b><a name="recovery">1.5 Was sind die Unterschiede zwischen Fehlerkorrektur auf Datei- und Abbild-Ebene?</a></b><p>

Optische Datenträger sind aus 2048 Bytes großen Sektoren aufgebaut.
Die meisten Sektoren davon werden zum Speichern von Dateien verwendet,
aber einige zusätzliche Sektoren sind nötig, 
um "Meta-Daten" wie z.B. die Struktur
von Unterverzeichnissen zu speichern.<br> 
Abbildung 1.5.1 (nachfolgend) zeigt wie ein Verzeichnis "Fotos" mit drei
Dateien "wald.jpg", "felsen.jpg" und 
"schutz.par" <a href="#note1"><sup>1)</sup></a> auf dem Datenträger
realisiert ist: Die Dateien werden auf die grünen bzw. blauen Sektoren
abgebildet. Ein weiterer rot markierter Sektor wird benötigt,
um die Struktur des "Fotos"-Verzeichnisses abzuspeichern.
<p>

<table width="100%"><tr><td align="center"><img src="images/metadata1.png" alt="Beziehung zwischen dem Dateisystem und den Datenträger-Sektoren"></td></tr></table><p>

<b>Nachteile der Fehlerkorrektur auf Dateisystem-Ebene bei optischen Datenträgern.</b><br>
Nehmen wir jetzt eine Dateisystem-basierte Fehlerkorrektur an. 
Die Datei "schutz.par" enthält Fehlerkorrektur-Informationen, mit denen
unlesbare Sektoren innerhalb der Dateien "wald.jpg" und "felsen.jpg"
wiederhergestellt werden können. Dies funktioniert nur solange wie
wir unlesbare Sektoren antreffen, die innerhalb von Dateien liegen.
Sobald Meta-Daten unlesbar werden, versagt die Fehlerkorrektur auf
Datei-Ebene. Betrachten Sie Abbildung 1.5.2: Wenn der rote Sektor
für das Unterverzeichnis "Fotos" unlesbar wird, verlieren Sie nicht nur das
Verzeichnis selbst, sondern auch alle darin enthaltenen Dateien.
Dies liegt an der logischen Struktur des ISO/UDF-Dateisystems.
Sobald die Meta-Daten des Verzeichnisses verloren sind, gibt es keine
Möglichkeit mehr festzustellen wie die grünen und blauen Sektoren
den Dateien zugeordnet sind. Daher haben wir einen kompletten Datenverlust
obwohl alle zu den Dateien gehörenden Sektoren noch lesbar sind.<p>

<table width="100%"><tr><td align="center"><img src="images/metadata2.png" alt="Unlesbare Sektoren mit Metadaten ergeben vollständigen Datenverlust"></td></tr></table><p>

<p>Bitte beachten Sie daß sich das Problem nicht lösen läßt indem man
die Datei "schutz.par" auf einem anderen Datenträger speichert.
Der Unterverzeichnis-Sektor ist immer noch nicht wiederherstellbar
da er nicht von den Fehlerkorrektur-Daten innerhalb von "schutz.par"
abgedeckt wird.<p>

<b>Vorteile der Fehlerkorrektur auf Abbild-Ebene bei optischen Datenträgern.</b><br>

dvdisaster betreibt Fehlerkorrektur auf der Abbild-Ebene.
Der Datenträger wird als ISO-Abbild gelesen und verarbeitet.
Das ISO-Abbild enthält alle Sektoren des Datenträgers, also auch diejenigen
die Meta-Daten des Dateisystems enthalten. Dementsprechend schützt die
Fehlerkorrektur von dvdisaster <i>alle Sektoren</i> innerhalb des
ISO-Abbildes, d.h. sowohl Dateien als auch Meta-Daten (z.B. Verzeichnisse)
werden wiederhergestellt. Abbildung 1.5.3 verdeutlicht den Bereich des
Abbild-basierten Schutzes.

<table width="100%"><tr><td align="center"><img src="images/metadata3.png" alt="Schutz auf der Abbild-Ebene"></td></tr></table><p>

Weder das Lesen des beschädigten ISO-Abbildes noch die Anwendung
der Fehlerkorrektur benötigen Informationen aus dem Dateisystem.
Solange das Laufwerk den Datenträger noch erkennt, wird dvdisaster
in der Lage sein die noch lesbaren Sektoren einzusammeln. Daher gibt es
im Gegensatz zum Datei-basierten Ansatz 
keine kritischen Einzelsektoren, die zum kompletten Ausfall der
Fehlerkorrektur führen können.

<table width="30%" cellpadding="0">
<tr><td class="hsep"></td></tr>
</table>
<a name="note1"><sup>1)</sup>
Das ist nicht als Angriff gegen das PAR/PAR2-Projekt gemeint. 
Carsten ist einfach der Meinung daß ein Schutz auf Datei-Ebene
auf optischen Datenträgern nicht funktioniert :-)</a>

<div class="talignr"><a href="#top">&uarr;</a></div><p>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
