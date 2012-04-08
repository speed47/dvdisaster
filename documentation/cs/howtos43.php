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
require("../include/screenshot.php");
begin_page();

howto_headline("Oprava bitových kopií disků", "Pokročilá nastavení", "images/create-icon.png");
?>První pokus o <a href="howtos42.php">přečtení poškozeného disku</a> obvykle poskytne dostatek dat k provedení opravy. Pokud ale ne, zkuste následující:<p><?php begin_screen_shot("Odhadnutí šance na úspěšné provedení opravy","adaptive-failure.png"); ?> <b>Odhadnutí šance na úspěšné provedení opravy</b><p>Prohlédněte si výstup procesu čtení. V sekci &quot;Zpracované sektory&quot; naleznete údaje o procentech čitelných sektorů a počtu sektorů nutných k opravě. Pomocí rozdílu těchto dvou hodnot (v případě tohoto příkladu 85.6% - 81.3% = 4.3%) můžete odhadnout pravděpodobnost získání dostatečného počtu sektorů pro provedení opravy:<p><?php end_screen_shot(); ?><table cellspacing="0" cellpadding="10">
<tr style="background-color:#c0ffc0;">
<td class="w10p" align="center" valign="top">&lt; 5%</td>
<td>Při použití dalších pokusů o přečtení máte velice dobrou šanci získat dostatek potřebných dat.</td></tr>
<tr style="background-color:#ffffc0;">
<td class="w10p" align="center" valign="top">5%-10%</td>
<td>Pokud máte několik mechanik s rozdílnými charakteristikami čtení, můžete požadovaná data získat pokud budete důkladní a trpěliví.</td></tr>
<tr style="background-color:#ffe0c0;">
<td class="w10p" align="center" valign="top">10%-20%</td>
<td>Máte problém. Pokud během následujících 2-3 pokusů o přečtení počet nečitelných sektorů výrazně nepoklesne pod 10%, je disk pravděpodobně neopravitelný.</td></tr>
<tr style="background-color:#ffc0c0;">
<td class="w10p" align="center" valign="top">> 20%</td>
<td>Příliš velká ztráta dat, disk je neopravitelný. Opakování této situace můžete předejít používáním dat pro opravu chyb s vyšší redundancí a zkrácením intervalů mezi kontrolami disků.</td></tr>
</table><p>Při provádění dodatečných pokusů o přečtení použijte jednotlivá níže uvedená nastavení jedno po druhém v uvedeném pořadí. Po každé změně nastavení proveďte jedno kompletní čtení, abyste zjistili, jak ovlivní výsledek (výsledky se také občas liší v závislosti na použité mechanice). Pokud už jste vyzkoušeli všechna jednotlivá nastavení, můžete zkusit jejich kombinace.<hr>

<?php begin_screen_shot("Provedení dalšího pokusu o přečtení","fix-prefs-read-attempts1.png"); ?><b>Provedení dalšího pokusu o přečtení</b><p>Neměňte žádná jiná nastavení kromě hodnoty nečitelných sektorů pro ukončení procesu čtení. Doporučené hodnoty jsou: 32 pro BD, 16 pro DVD a 0 pro CD (použijte zeleně označený posuvník). Proveďte další pokus o přečtení s tímto nastavením. Pokusy o přečtení můžete opakovat tak dlouho, dokud při nich přibývá významný počet nových sektorů.<p><b>Tip:</b> Mechaniku nechte mezi jednotlivými pokusy odpočinout. Před každým pokusem vysuňte a znovu zasuňte disk, je totiž možné, že pak disk skončí v lepší pozici a zlepší se jeho čitelnost.<p><?php end_screen_shot(); ?><hr><b>Doplnění bitové kopie pomocí různých mechanik</b><p>Další pokusy proveďte pomocí jiné mechaniky. Přesuňte bitovou kopii na jiný počítač a zkuste, zda se vám na něm nepodaří přečíst více sektorů.<p><hr>

<?php begin_screen_shot("Zvýšení počtu pokusů o přečtení","fix-prefs-read-attempts2.png"); ?><b>Zvýšení počtu pokusů o přečtení</b><p><i>Pro všechny typy disků (CD, DVD, BD):</i><p>Nastavte minimální počet pokusů o přečtení sektoru na 5 a maximální na 9 (označeno zeleně).<p><i>Pouze pro CD:</i><p>Některé mechaniky jsou schopné částečně přečíst poškozené sektory na CD. Aktivujte &quot;Ukládání nezpracovaných sektorů&quot; a zadejte složku do které mají být fragmenty poškozených sektorů ukládány (označeno žlutě). Pokud je získán dostatek fragmentů poškozeného sektoru, je možné ho díky těmto informacím opravit. <?php end_screen_shot(); ?> <?php begin_screen_shot("Opakované pokusy o přečtení","fix-reread-dvd.png"); ?> <i>Kontrola výsledků vícenásobných pokusů o přečtení (CD, DVD, BD):</i><br>Ne u všech mechanik dojde po zvýšení pokusů o přečtení ke zlepšení čitelnosti. Sledujte hlášení typu &quot;Sektor ..., pokus x: úspěšný&quot; (označeno žlutě). Ty znamenají, že se mechanice podařilo po několika opakovaných čteních sektor přečíst. Pokud takové hlášení není nikdy zobrazeno, zvýšení počtu pokusů o přečtení se pro danou mechaniku nevyplatí. <?php end_screen_shot(); ?> <a name="21h"></a> <i>Kontrola částečného čtení poškozených CD sektorů:</i><br>Po zpracování celého disku se podívejte do výše zadaného adresáře (v příkladu /var/tmp/raw). Pokud nebyly vytvořeny žádné soubory, mechanika pravděpodobně nepodporuje požadovaný režim čtení. Pokud ale máte několik mechanik schopných soubory fragmentů vytvářet, použijte pro všechny stejný pracovní adresář. Sběr fragmentů poškozených sektorů za pomoci různých mechanik zvyšuje pravděpodobnost jejich opravy.<p><?php begin_screen_shot("Použití jiného režimu přímého čtení","fix-prefs-drive2.png"); ?> <i>Použijte pro CD jiný režim přímého čtení:</i><br>Na některých mechanikách režim přímého čtení &quot;20h&quot; nefunguje. Proto proveďte další pokus o přečtení v režimu přímého čtení &quot;21h&quot; (viz snímek obrazovky). Znovu zkontrolujte zda byly vytvořeny nějaké soubory fragmentů sektorů.<?php end_screen_shot(); ?> <!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>