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
require("../include/footnote.php");
begin_page();

howto_headline("Vytvoření souborů pro opravu chyb", "Uchovávání", "images/create-icon.png");
?>

<!-- Insert actual page content below -->
<h3 class="top">Rady pro uchovávání souborů pro opravu chyb</h3>CD/DVD/BD jsou v současné době cenově nejvýhodnějším formátem médií pro výměnu dat. Proto pravděpodobně zvažujete pro uchovávání souborů pro opravu chyb jejich použití.<p>Na tom není nic špatného, ale musíte mít na paměti, že jak vaše data, tak související ochranná data jsou pak uložena na médiích se stejnou spolehlivostí. Pokud se již setkáte s chybami na datovém disku, je velice pravděpodobné, že disk obsahující související data pro opravu chyb je poškozen také. Oba disky byly přece vypáleny ve stejnou dobu a mají stejné charakteristiky stárnutí.<p><table width="100%"><tr><td class="vsep"></td>
<td> </td>
<td>Ačkoliv to může být pro někoho překvapující, nemůže být zaručeno, že soubor pro opravu chyb zůstane použitelný pokud je uložen na poškozeném disku - zde je <a href="qa32.php#file">technické vysvětlení problému</a>.</td></tr></table><p>Soubory pro opravu chyb je proto nutné chránit stejně jako normální data. Přesněji řečeno, disk obsahující soubory pro opravu chyb musí být také chráněn daty pro opravu chyb. Existují dva způsoby jak toho dosáhnout:<ol>
<li>Uložení souborů pro opravu chyb na samostatných discích:<p>Použijte samostatné disky pouze k ukládání dat pro opravu chyb. Pokud disk soubory pro opravu chyb nezaplníte více než z 80%, můžete ho <a href="howtos30.php">rozšířit daty pro opravu chyb</a>. To vám umožní soubory pro opravu chyb opravit pokud v budoucnosti dojde k poškození disku.<p></li>

<li>Uložení souborů pro opravu chyb na dalším disku v sekvenci:<p>Možná disky používáte k inkrementálním zálohám. V tom případě počkejte než budete mít dostatek dat k zaplnění prvního disku. Zapište tato data jako vždy a vytvořte pro ně soubor pro opravu chyb. Tento soubor pro opravu chyb pak přidejte do příští zálohy. Po vypálení této další zálohy pak vytvořte soubor pro opravu chyb i pro ni a přidejte ho do další zálohy a takto pokračujte dále. Všechny disky v řadě jsou tak chráněny daty pro opravu chyb (se souborem pro opravu chyb pro poslední disk uloženým na pevném disku do doby než bude vypálena další záloha).<p>Samozřejmě, i přesto může udeřit Murphyho zákon a mohou se poškodit všechny disky v řadě. V tom případě bude třeba opravit všechny disky v řadě, tím nejnovějším počínaje ;-)</li>
</ol>

<!-- do not change below -->

<?php

# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
