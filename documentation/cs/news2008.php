<?php
# dvdisaster: English homepage translation
# Copyright (C) 2004-2012 Carsten Gnörlich
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

if($news_flash == 0) 
{  require("../include/dvdisaster.php");
   begin_page();
}

$news_counter = 0;

news_headline("Archiv novinek za rok 2008");

news_item("30.03.2008", "Vydán dvdisaster 0.71.26", "Tato verze obsahuje nový dialog s informacemi o vloženém disku (nabídka Nástroje/Informace o disku), společně s prací na této nové funkci byla také změněna detekce typu disku (CD/DVD/BD). Nahlaste proto prosím jakékoliv nekompatibility s dříve fungujícími mechanikami. Vylepšena podpora pro BD disky.", 5, "2008-03-30T00:00:00Z", "2008-03-30T00:00:00Z");

news_item("05.03.2008", "Opraveny problémy s předchozími verzemi pro Windows (0.70.6 / 0.71.25)", "Ve verzi 0.70.6 odstraněna podpora pro lokalizované názvy souborů, protože způsobovala problémy s podporou velkých souborů ve Windows. Nová obslužná rutina pro lokalizované názvy souborů bude otestována ve zkušební verzi 0.71.25.", 4, "2008-03-05T00:00:00Z", "2008-03-05T00:00:00Z");

news_item("03.03.2008", "Oops - ve verzích 0.70.5 a 0.71.24 nefungují ve Windows bitové kopie &gt;2GB", "Oprava pro lokalizované názvy souborů uvedená ve verzích 0.70.5 a 0.71.24 způsobila ve Windows problémy se zpracováním bitových kopií &gt; 2GB. Očekávejte brzké uvedení opravených verzí.", 3, "2008-03-03T00:00:00Z", "2008-03-03T00:00:00Z");

news_item("24.02.2008", "dvdisaster 0.70.5 / 0.71.24 opravuje problémy v nových verzích Linuxu", "Byl opraven problém který při použití novějších jader Linuxu mohl za určitých okolností způsobit zamrznutí systému. Pokud máte kernel 2.6.17 nebo novější, proveďte aktualizaci, je však také možné, že se chyba vyskytuje i u starších jader.<p> S vydáním dvdisaster 0.71.24 také započal přepis online dokumentace včetně ruského překladu, jehož autorem je Igor Gorbounov. <i>Anglická dokumentace není v současné době ani zdaleka dokončena. Mějte s námi trpělivost, brzy s tím snad pohneme.</i>", 2, "2008-02-24T00:00:00Z", "2008-02-24T00:00:00Z");

if($news_flash == 0) 
   end_page();
?>
