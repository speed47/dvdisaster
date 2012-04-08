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

<h3 class="top">Ostatní zdroje informací</h3>Online dokumentace kterou právě čtete je dodávána jako součást programových balíčků dvdisaster. Není třeba ji stahovat samostatně.<p>K dispozici jsou další dodatečné materiály:<p><b>Specifikace RS03</b><p>RS03 je nový formát kódování dat pro opravu chyb pro budoucí verze dvdisaster, který je schopen pro své výpočty využívat několik jader procesoru. To není u současných implementací RS01 a RS02 možné díky omezením jejich vnitřní struktury.<p>K diskusi je nyní k dispozici <a href="http://dvdisaster.net/papers/rs03.pdf">návrh RS03 specifikace (rs03.pdf)</a>. Specifikace není finální.<p>Čtení dokumentace RS03 vyžaduje znalosti z oblasti teorie kódování. Není zamýšlena jako dokumentace pro koncové uživatele.<!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>