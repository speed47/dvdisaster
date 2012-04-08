<?php
# dvdisaster: English homepage translation
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

<h3 class="top">Technické parametry opravy chyb</h3>Tato stránka vysvětluje základní myšlenky stojící za návrhem ochrany dat pomocí dvdisaster, abyste se mohli rozhodnout, zda odpovídá vašim požadavkům na zabezpečení dat. Pokud máte pochybnosti, neměli byste používat dvdisaster, nebo byste měli nasadit dodatečné strategie zálohy dat.<p><b>Metoda opravy chyb.</b> dvdisaster využívá <a href="http://en.wikipedia.org/wiki/Reed-Solomon_error_correction">Reed-Solomon</a> kód spolu s algoritmem opravy chyb optimalizovaným pro ošetření výmazů. Implementace čerpá většinu inspirace a programového kódu z vynikající <a href="http://www.ka9q.net/code/fec/">Reed-Solomon knihovny</a> kterou vytvořil <a href="http://www.ka9q.net/">Phil Karn</a>.<p>Při použití <a href="howtos22.php#ecc">výchozího nastavení</a> <a href="howtos20.php">souboru pro opravu chyb</a> je 223 sektorů disku zkombinováno do jednoho bloku kódu pro opravu chyb (ECC). Chyby čtení jsou považovány za &quot;výmazy&quot;; s pomocí jednoho ECC bloku je proto možné opravit maximálně 32 poškozených sektorů disku <sup><a href="#footnote1">*)</a></sup>.<p>Poloha každého z těchto 223 sektorů je volena tak, aby byly na povrchu disku rovnoměrně rozptýleny. Před dosažením limitu 32 chyb na ECC blok<sup><a href="#footnote1">*)</a></sup> je tak možno opravit i rozsáhlé spojité oblasti poškozených sektorů. Tento typ poškození je běžný u stárnoucích disků u kterých dochází k degeneraci vnější části disku a u škrábanců souběžných s datovou spirálou.<p>U radiálních nebo diagonálních škrábanců je předpokládáno, že je bude CD/DVD mechanika schopna opravit sama. Pokud ne, použitá strategie opravy chyb se v tomto případě chová neutrálně (ani mimořádně dobře ani mimořádně špatně).<p><b>Omezení opravy chyb.</b>   V nejhorším případě stačí k zabránění úplné obnovy dat 33 poškozených sektorů<sup><a href="#footnote1">*)</a></sup>. Aby však bylo tohoto efektu dosaženo, musely by chyby být po povrchu disku rozmístěny tak, aby se všechny vyskytovaly v jednom ECC bloku - takové rozmístění je velmi nepravděpodobné.<br> Testy ukázaly, že u stárnoucích disků bylo ve většině případů limitu 33 chyb na ECC blok<sup><a href="#footnote1">*)</a></sup> dosaženo až při poškození 10% z celkového počtu sektorů. <br> Škrábance mohou způsobit, že bude této hranice dosaženo dříve, je proto doporučeno, provádět pravidelně vizuální kontroly disků. Disky s chybami čtení způsobenými škrábanci, by měly být okamžitě vyměněny.<p><b>Hardwarová omezení.</b>   Většina mechanik nerozpozná disk, pokud je poškozena zaváděcí oblast před prvním sektorem (u středového otvoru). Obnova dat v takovém případě není možná.<p>Je <i>nemožné</i> použitím dvdisaster zlepšit spolehlivost nekvalitních disků. Levné disky mohou během pár dní degradovat do stavu, který přesáhne schopnosti kódu pro opravu chyb.<p><pre> </pre>
<table width="50%"><tr><td><hr></td></tr></table><span class="fs"> <a name="footnote1"><sup>*)</sup></a> Limit 32 opravitelných chyb na ECC blok je výchozím nastavením Je možné <a href="howtos22.php#ecc">zvolit jiné hodnoty</a> pro zvýšení nebo snížení schopnosti opravy chyb. Pokud <a href="howtos30.php">jsou data pro opravu chyb umístěna přímo na disk</a>, závisí počet opravitelných sektorů na množství volného místa na disku. </span> <!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>