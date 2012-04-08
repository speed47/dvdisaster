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

<h3 class="top">Tipy pro uchovávání souborů pro opravu chyb</h3>V současné době je málo přenosných technologií médií, které mohou být cenově efektivní alternativou k CD/DVD/BD. Pravděpodobně proto nebudete CD/DVD/BD používat jen pro archivaci dat, ale také pro uchovávání k nim náležejících souborů pro opravu chyb.<p>Na tom není nic špatného, ale nesmíte zapomínat, že jak vaše data, tak soubory pro opravu chyb jsou uchovávány na médiích se stejným stupněm čitelnosti. Pokud dojde k výskytu chyb čtení u archivovaných dat, musíte být připraveni na to, že i disk s k nim náležejícím souborem pro opravu chyb již také nemusí být zcela čitelný.<p>Je proto důležité chránit data pro opravu chyb stejně jako ostatní data<sup><a href="#footnote1">*)</a></sup>. Toho je nejlépe dosaženo integrací dat pro opravu chyb do vašeho běžného plánu zálohování. Zde jsou dva návrhy:<p><b>1. Uchovávání souborů pro opravu chyb na vyhrazeném disku:</b><p>Pokud se rozhodnete uchovávat soubory pro opravu chyb na vyhrazeném disku, je <a href="qa32.php#eccfile">důležité</a> tyto disky také chránit prostřednictvím dvdisaster. By jste se vyhnuli nekonečnému řetězu (soubory pro opravu chyb pro disk se soubory pro opravu chyb pro ...), zkuste následující:<p>Uvažujme, že na každý disk je možné uložit pět souborů pro opravu chyb. Zapište prvních pět souborů pro opravu chyb na disk a vytvořte soubor pro opravu chyb pro tento disk. Nyní tento soubor pro opravu chyb uložte spolu s jinými čtyřmi soubory pro opravu chyb na další disk. Pokud budete pokračovat tímto způsobem, budou všechny soubory pro opravu chyb s výjimkou těch z posledního disku (které mohou být zatím uchovány na pevném disku) chráněny prostřednictvím dvdisaster.<p><b>2. Uchování souboru pro opravu chyb na následujícím disku série:</b><p>Pokud disky zcela nezaplníte (například máte méně než 4GB dat na jednovrstvé DVD), můžete soubor pro opravu chyb uchovávat na následujícím disku série.<p><pre> </pre>
<table width="50%"><tr><td><hr></td></tr></table><span class="fs"> <a name="footnote1"><sup>*)</sup></a> Namísto vytvoření souboru pro opravu chyb můžete také zvolit <a href="howtos30.php">rozšířenou bitovou kopii</a> využívající RS02 nebo RS03. </span> <!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>