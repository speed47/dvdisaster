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

<h3 class="top"><a name="top">Technické otázky</a></h3><a href="#nls">2.1 Do kterých jazyků byl již program přeložen?</a><p><a href="#media">2.2 Které typy disků jsou podporovány?</a><p><a href="#filesystem">2.3 Které souborové systémy jsou podporovány?</a><p><hr><p><b><a name="nls">2.1 Do kterých jazyků byl již program přeložen?</a></b><p>Aktuální verze dvdisaster obsahuje překlad uživatelského rozhraní do následujících jazyků:<p><table>
<tr><td>   </td><td>Czech</td><td>--</td><td>kompletní</td></tr>
<tr><td></td><td>English</td><td>--</td><td>kompletní</td></tr>
<tr><td>   </td><td>German</td><td>--</td><td>kompletní</td></tr>
<tr><td>   </td><td>Italian</td><td>--</td><td>do verze 0.65</td></tr>
<tr><td></td><td>Portuguese</td><td>--</td><td>kompletní</td></tr>
<tr><td>   </td><td>Russian</td><td>--</td><td>kompletní</td></tr>
<tr><td>   </td><td>Swedish</td><td>--</td><td>do verze 0.70</td></tr>
</table><p>Překlady do dalších jazyků jsou vítány!<p>dvdisaster automaticky detekuje jazyková nastavení operačního systému. Pokud lokální jazyk není podporován, bude použita angličtina. Jiný jazyk může být vybrán pomocí nastavení systémových proměnných.<p>Příklad pro příkazový bash a němčinu:<pre>export LANG=de_DE</pre>Pokud nejsou správně zobrazeny speciální znaky jako např. německé přehlasované hlásky, zkuste následující:<p><tt>export OUTPUT_CHARSET=iso-8859-1</tt> (X11, XTerm)<div class="talignr"><a href="#top">↑</a></div><b><a name="media">2.2 Které typy disků jsou podporovány?</a></b><p>dvdisaster podporuje zapisovatelné disky CD, DVD a BD. <br>Disky obsahující více sekcí nebo ochranu proti kopírování <i>nemohou</i> být používány.<p>Použitelné typy disků:<p><b>CD-R, CD-RW</b><p><ul>
 <li>Podporována pouze datová CD.</li>
</ul><b>DVD-R, DVD+R</b><p><ul>
<li>Žádná další známá omezení.</li>
</ul><b>DVD-R DL, DVD+R DL (dvouvrstvá)</b><ul>
<li>Mechanika musí být schopna <a href="qa20.php#dvdrom">rozeznat typ disku</a>. To je většinou případ pouze mechanik které jsou také schopny vypalovat dvouvrstvé disky.</li>
</ul><b>DVD-RW, DVD+RW</b><p><ul>
<li>Některé mechaniky hlásí špatnou <a href="qa20.php#rw">velikost</a>.<br>Lze obejít zjištěním velikosti ze systému souborů ISO/UDF nebo údajů ECC/RS02 dat.</li></ul><b>DVD-RAM</b><p><ul>
<li>Použitelné pouze při zápisu ISO/UDF jako u DVD-R/-RW;</li>
<li>Nepoužitelné při použití jako vyjímatelný disk (paketový zápis).</li>
<li>Podobně jako uvedeno výše problém s rozpoznáním <a href="qa20.php#rw">velikosti bitové kopie</a>.</li>
</ul><b>BD-R, BD-RW</b><p><ul>
<li>Žádná známá omezení, ale bylo provedeno jen minimum testů dvouvrstvých disků (50GB).</li>
</ul><b>Nepoužitelné typy</b> (nelze vytvořit bitovou kopii):<p>BD-ROM (originální BD), DVD-ROM (originální DVD), CD-Audio a CD-Video.<div class="talignr"><a href="#top">↑</a></div><p><b><a name="filesystem">2.3 Které souborové systémy jsou podporovány?</a></b><p>dvdisaster pracuje výhradně na úrovni bitových kopií, ke kterým je přistupováno sektor po sektoru. Nezáleží proto na tom, jaký systém souborů byl u disku použit.<p>Protože dvdisaster nezná, ani nepoužívá strukturu systému souborů, nemůže opravit logické chyby na úrovni systému souborů. Nemůže proto obnovit ztracené nebo smazané soubory.<div class="talignr"><a href="#top">↑</a></div><p><!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>