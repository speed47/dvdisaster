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

<h3 class="top">Testování kompatibility bitové kopie</h3><b>Proč dvdisaster používá bitové kopie ISO.</b> Některé funkce dvdisaster pracují se soubory bitových kopií uloženými na pevném disku. CD/DVD/BD mechaniky jsou pro použité vzorce přístupu příliš pomalé a rychle by se opotřebovali. Pevné disky ale byly pro podobný typ přístupu navrženy a provádí ho rychle a bez opotřebení.<p><b>Testování kompatibility bitové kopie je důležité.</b> Během práce s dvdisaster můžete (a někdy musíte) používat bitové kopie ISO vytvořené v jiných aplikacích. Formát bitových kopií ISO nemá bohužel pevně daný standard. Většina programů vytvoří při použití formátu ISO identické bitové kopie, je ale lepší si ověřit, zda byla vytvořena použitelná bitová kopie: Zpracování bitových kopií jiného formátu než ISO vytvoří nepoužitelná data pro opravu chyb. Zvláště formáty jako <b>NRG nejsou vhodné</b> pro zpracování pomocí dvdisaster.<p><b>Možné scénáře.</b> Následující situace vyžadují použití bitové kopie ISO z jiné aplikace:<p><b>a) Vytvoření dat pro opravu chyb pro bitovou kopii ISO vytvořenou pomocí vypalovacího programu</b><p>Pro vytvoření bitové kopie ISO je použit vypalovací program pro CD/DVD/BD. Tato bitová kopie je použita pro vytvoření souboru pro opravu chyb a je vypálena na disk. Pokud daný vypalovací program používáte s dvdisaster poprvé, ujistěte se, že <a href="howtos91.php">bitová kopie byla na disk vypálena bez jakýchkoliv úprav</a>.<p><b>b) Rozšíření bitové kopie ISO o data pro opravu chyb</b><p>dvdisaster přidá do bitové kopie &quot;neviditelná&quot; (pro minimalizaci interferencí s jinými aplikacemi) data pro opravu chyb. Je možné, že některé vypalovací programy data pro opravu chyb nezapíší správně. Po prvním vypalování se ujistěte, že váš vypalovací program při použití rozšířených bitových kopií <a href="howtos92.php">správně zapíše data pro opravu chyb</a>.<!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>