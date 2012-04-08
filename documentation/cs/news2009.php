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

news_headline("Archiv novinek za rok 2009");

news_item("01.10.2009", "Zhodnocení potenciální zranitelnosti v GTK knihovně pro Windows", "Verze dvdisaster 0.70.x/0.72.x pro Windows jsou dodávány se zastaralou knihovnou GTK která obsahuje zranitelnosti ve funkcích pro zpracování obrázků. Pro využití této zranitelnosti musí být z externího zdroje načteny upravené obrázky. Protože dvdisaster neobsahuje/nepoužívá dané funkce, nelze tyto zranitelnosti považovat za hrozbu.<p> Nedoporučujeme nahrazovat GTK ve verzi 0.70.x/0.72.x dvdisaster novější verzí protože některá rozhraní byla v novějších verzích GTK změněna. Je pravděpodobné, že nahrazení GTK způsobí chyby.<p>Verze dvdisaster 0.73.1 pro Windows bude mít aktualizovaná rozhraní a bude dodávána s aktuální verzí GTK.<p> Děkuji uživatelům, kteří mě na tuto situaci upozornili.", 13, "2009-10-01T00:00:00Z", "2009-10-01T00:00:00Z");

news_item("10.08.2009", "Popis projektu na SourceForge může být zastaralý", "Díky změnám ve funkcionalitě SourceForge aktuálně nemůžeme včas aktualizovat Novinky, Stahování a obsah CVS. Používejte proto raději příslušné sekce této stránky (http://dvdisaster.net).", 12, "2009-08-10T00:00:00Z", "2009-08-10T00:00:00Z");

news_item("08.08.2009", "Vydán dvdisaster 0.72.1", "Pablo Almeida vytvořil portugalský překlad programu. Přidána funkce která má předejít zamrzání Win XP u některých kombinací CD-RW/mechanik.<p> <i>Aktualizace: funkce není kompletní. Pokud problémy přetrvávají vyzkoušejte verzi 0.79.x.</i>", 11, "2009-08-08T00:00:00Z", "2010-02-06T00:00:00Z");

news_item("04.07.2009", "Vydán dvdisaster 0.72", "První stabilní verze větve 0.72. Igor Gorbounov dokončil ruskou online dokumentaci. Byly opraveny některé chyby nalezené v první RC verzi.<p>", 10, "2009-07-04T00:00:00Z", "2009-07-04T00:00:00Z");

news_item("14.04.2009", "Přidáno: dvdisaster 0.72.rc1 pro Mac OS X", "Nativní verze pro Mac OS X má ještě pořád trochu <a href=\"download30.php#mac\">problémy</a> s uživatelským rozhraním: port potřebné GTK+ knihovny je stále ještě v rané fázi vývoje. Ale díky dnešní aktualizaci se již dvdisaster stává na Macu použitelný...", 9, "2009-04-14T00:00:00Z", "2009-04-14T00:00:00Z");

news_item("11.04.2009", "Vydán dvdisaster 0.72.rc1", "Je k dispozici první RC stabilní větve 0.72. Mezi novými funkcemi najdete mimo jiné podporu pro Blu-Ray disky, přímé čtení a C2 kontrolu pro CD. Více informací naleznete na <a href=\"download.php\">stránce se soubory ke stažení</a>.", 8, "2009-04-11T00:00:00Z", "2009-04-11T00:00:00Z");

news_item("08.03.2009", "Vydán dvdisaster 0.71.28", "Tato verze opravuje \"ilegální instrukci\" na x86 počítačích nepodporujících SSE2. Také opraveno několik málo významných chyb.", 7, "2009-03-08T00:00:00Z", "2009-03-08T00:00:00Z");

news_item("18.01.2009", "Vydán dvdisaster 0.71.27", "Dokumentace je konečně kompletní. Opraveny některé pády a nekompatibility s méně obvyklými disky a mechanikami. Tato verze obsahuje poslední větší interní změny plánované pro verze 0.71.x, takže pokud bude vše podle plánu, další verze by měla být stabilní RC verze.", 6, "2009-01-18T00:00:00Z", "2009-01-18T00:00:00Z");

if($news_flash == 0) 
   end_page();
?>
