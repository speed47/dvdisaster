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
$show_all=$_GET["showall"];
?>

<!-- Insert actual page content below -->

<h3 class="top">Stáhnout dvdisaster</h3>

<!--
<table width="100%">
  <tr>
    <td>dvdisaster je k dispozici pro <a href="download10.php">aktuální verze</a> operačních systémů FreeBSD, GNU/Linux, Mac OS X(Darwin), NetBSD a Windows. Je poskytován jako <a href="http://www.germany.fsfeurope.org/documents/freesoftware.en.html">volný software</a> pod <a href="http://www.gnu.org/licenses/gpl-3.0.txt">GNU General Public License v3</a>.</td>
    <td class="w127x" valign="top"><img src="../images/gplv3-127x51.png" alt="Logo GPLv3" width="127"></td>
  </tr>
</table>
-->

dvdisaster je k dispozici pro <a href="download10.php">aktuální verze</a> operačních systémů FreeBSD, GNU/Linux, Mac OS X(Darwin), NetBSD a Windows. Je poskytován jako <a href="http://www.germany.fsfeurope.org/documents/freesoftware.en.html">volný software</a> pod <a href="http://www.gnu.org/licenses/gpl-2.0.txt">GNU General Public License v2</a>.
<p>
Z níže uvedeného seznamu si ke stažení vyberte zdrojový kód, nebo binární verzi. Pro ověření pravosti a nepoškozenosti balíčků jsou k dispozici jejich <a href="download20.php">digitální podpisy</a>.<p><ul>
<li>Balíčky zdrojových kódů obsahují soubor <tt>INSTALL</tt> s návodem pro kompilaci.</li>
<li>Pro Mac OS X je k dispozici ZIP archiv jehož instalaci provedete jednoduchým rozbalením do vámi vybraného adresáře. Prohlédněte si také <a href="download30.php#mac">speciální tipy pro Mac OS X</a>.</li>
<li><a href="download30.php#win">Instalaci</a> binární verze pro Windows proveďte jednoduchým spuštěním instalačního programu a postupujte podle zobrazovaných instrukcí.</li>
</ul>

<?php
if(!strcmp($have_experimental, "yes"))
{ ?><b>Alpha (nestabilní) verze</b> - nová a experimentální pro zkušené uživatele!<p>Uvítáme uživatele testující budoucí verze dvdisaster, ale testující by si měli uvědomit, že mohou obsahovat chyby a mít problémy s kompatibilitou. Aktuální nestabilní verze je <a href="download40.php"><?php echo $cooked_version?></a>.<p><?php
}
?> <b>Stabilní verze</b> - doporučeno pro nové a nezkušené uživatele.<p><a name="download"></a><table class="download" cellpadding="0" cellspacing="5">
<tr><td><b>dvdisaster-0.72</b></td><td align="right">2012-07-04</td></tr>
<tr><td colspan="2" class="hsep"></td></tr>
<tr><td colspan="2"><table>
    <tr><td align="right">  Zdrojový kód (všechny operační systémy): </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.4.tar.bz2">dvdisaster-0.72.4.tar.bz2</a></td></tr>
    <tr><td align="right">Digitální podpis </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.4.tar.bz2.gpg">dvdisaster-0.72.4.tar.bz2.gpg</a></td></tr>

<?php
if($mode == "www")
    echo "<tr><td align=\"right\">Kontrolní součet MD5:&nbsp;</td><td>4eb09c1aa3cdbc1dafdb075148fb471d</td></tr>";
?>
    <tr><td><pre> </pre></td><td></td></tr>


    <tr><td align="right">  Zdrojový kód (všechny operační systémy): </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.3.tar.bz2">dvdisaster-0.72.3.tar.bz2</a></td></tr>
    <tr><td align="right">Digitální podpis </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.3.tar.bz2.gpg">dvdisaster-0.72.3.tar.bz2.gpg</a></td></tr>

<?php
if($mode == "www")
    echo "<tr><td align=\"right\">Kontrolní součet MD5:&nbsp;</td><td>4eb09c1aa3cdbc1dafdb075148fb471d</td></tr>";
?>
    <tr><td colspan="2" class="esep"></td></tr>

    <tr><td align="right">Binární soubor pro Mac OS X 10.6 / x86: </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.3.app.zip">dvdisaster-0.72.3.app.zip</a> -- pročtěte si prosím nejdříve tyto <a href="download30.php#mac">tipy</a></td></tr>
    <tr><td align="right">Digitální podpis </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.3.app.zip.gpg">dvdisaster-0.72.3.app.zip.gpg</a></td></tr>

<?php
if($mode == "www")
    echo "<tr><td align=\"right\">Kontrolní součet MD5:&nbsp;</td><td>38389bbbeb0d259a3f0a8df89b363f72</td></tr>";
?>
    <tr><td colspan="2" class="esep"></td></tr>

    <tr><td align="right">Binární soubor pro Windows: </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.3-setup.exe">dvdisaster-0.72.3-setup.exe</a></td></tr>
    <tr><td align="right">Digitální podpis </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.3-setup.exe.gpg">dvdisaster-0.72.3-setup.exe.gpg</a></td></tr>

<?php
if($mode == "www")
    echo "<tr><td align=\"right\">Kontrolní součet MD5:&nbsp;</td><td>b6861ba1e8de6d91a2da5342a14870e0</td></tr>";
?>
    <tr><td colspan="2"></td></tr>

<?php
  if($show_all == 0) {
?>
    <tr><td colspan="2"><a href="download.php?showall=1#download">Zobrazit starší verze z 0.72 větve</a></td></tr>
<?php
  }
  else {
?> 
   <tr><td colspan="2"><a href="download.php?showall=0#download">Skrýt starší verze z 0.72 větve</a></td></tr>

   <tr><td colspan="2"></td></tr>
   <tr><td></td><td>Verze 0.72.2</td></tr>
    <tr><td align="right">  Zdrojový kód (všechny operační systémy): </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.2.tar.bz2">dvdisaster-0.72.2.tar.bz2</a></td></tr>
    <tr><td align="right">Digitální podpis </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.2.tar.bz2.gpg">dvdisaster-0.72.2.tar.bz2.gpg</a></td></tr>

    <tr><td align="right">Kontrolní součet MD5:&nbsp;</td><td>312bceef3bf9c0754cf633ed3b12eb71</td></tr>
    <tr><td colspan="2" class="esep"></td></tr>

    <tr><td align="right">Binární soubor pro Mac OS X 10.5 / x86: </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.2.app.zip">dvdisaster-0.72.2.app.zip</a> -- pročtěte si prosím nejdříve tyto <a href="download30.php#mac">tipy</a></td></tr>
    <tr><td align="right">Digitální podpis </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.2.app.zip.gpg">dvdisaster-0.72.2.app.zip.gpg</a></td></tr>

    <tr><td align="right">Kontrolní součet MD5:&nbsp;</td><td>52243c1fafb9d2e496b6eb318c3e534f</td></tr>
    <tr><td colspan="2" class="esep"></td></tr>

    <tr><td align="right">Binární soubor pro Windows: </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.2-setup.exe">dvdisaster-0.72.2-setup.exe</a></td></tr>
    <tr><td align="right">Digitální podpis </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.2-setup.exe.gpg">dvdisaster-0.72.2-setup.exe.gpg</a></td></tr>

    <tr><td align="right">Kontrolní součet MD5:&nbsp;</td><td>f80258d27354061fd9e28850ec4701a6</td></tr>

   <tr><td colspan="2"></td></tr>
   <tr><td></td><td>Verze 0.72.1</td></tr>
    <tr><td align="right">  Zdrojový kód (všechny operační systémy): </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.1.tar.bz2">dvdisaster-0.72.1.tar.bz2</a></td></tr>
    <tr><td align="right">Digitální podpis </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.1.tar.bz2.gpg">dvdisaster-0.72.1.tar.bz2.gpg</a></td></tr>
    <tr><td align="right">Kontrolní součet MD5: </td>
        <td>4da96566bc003be93d9dfb0109b4aa1d</td></tr>
    <tr><td colspan="2" class="esep"></td></tr>

    <tr><td align="right">Binární soubor pro Mac OS X 10.5 / x86: </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.1.app.zip">dvdisaster-0.72.1.app.zip</a> -- pročtěte si prosím nejdříve tyto <a href="download30.php#mac">tipy</a></td></tr>
    <tr><td align="right">Digitální podpis </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.1.app.zip.gpg">dvdisaster-0.72.1.app.zip.gpg</a></td></tr>
    <tr><td align="right">Kontrolní součet MD5: </td>
        <td>924b5677f69473b6b87991e01779a541</td></tr>
    <tr><td colspan="2" class="esep"></td></tr>

    <tr><td align="right">Binární soubor pro Windows: </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.1-setup.exe">dvdisaster-0.72.1-setup.exe</a></td></tr>
    <tr><td align="right">Digitální podpis </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.1-setup.exe.gpg">dvdisaster-0.72.1-setup.exe.gpg</a></td></tr>
    <tr><td align="right">Kontrolní součet MD5: </td>
        <td>34d062ddebe1a648e808d29ca4e9879f</td></tr>

   <tr><td colspan="2"></td></tr>
   <tr><td></td><td>Verze 0.72</td></tr>
    <tr><td align="right">  Zdrojový kód (všechny operační systémy): </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.tar.bz2">dvdisaster-0.72.tar.bz2</a></td></tr>
    <tr><td align="right">Digitální podpis </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.tar.bz2.gpg">dvdisaster-0.72.tar.bz2.gpg</a></td></tr>
    <tr><td align="right">Kontrolní součet MD5: </td>
        <td>efa35607d91412a7ff185722f270fb8a</td></tr>
    <tr><td colspan="2" class="esep"></td></tr>

    <tr><td align="right">Binární soubor pro Mac OS X 10.5 / x86: </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.app.zip">dvdisaster-0.72.app.zip</a> -- pročtěte si prosím nejdříve tyto <a href="download30.php#mac">tipy</a></td></tr>
    <tr><td align="right">Digitální podpis </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.app.zip.gpg">dvdisaster-0.72.app.zip.gpg</a></td></tr>
    <tr><td align="right">Kontrolní součet MD5: </td>
        <td>1f28385b2b6d64b664fd416eb4c85e80</td></tr>
    <tr><td colspan="2" class="esep"></td></tr>

    <tr><td align="right">Binární soubor pro Windows: </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72-setup.exe">dvdisaster-0.72-setup.exe</a></td></tr>
    <tr><td align="right">Digitální podpis </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72-setup.exe.gpg">dvdisaster-0.72-setup.exe.gpg</a></td></tr>
    <tr><td align="right">Kontrolní součet MD5: </td>
        <td>cc8eb2af384917db8d6d983e1d4aac69</td></tr>
<?php
  }
?>
  </table>
</td></tr>
<tr><td colspan="2" class="hsep"></td></tr>
<tr><td colspan="2">Nejdůležitější změny v této verzi:<p><ul>
<li>Podpora pro <a href="qa10.php#media">Blu-Ray disky</a></li>
<li>&quot;Přímé&quot; čtení a kontrola C2 pro CD disky</li>
<li>Volitelný počet pokusů o přečtení</li>
<li>První nativní balíček verze aplikace pro Mac OS X</li>
<li>NetBSD port od Sergey Svishchev</li>
<li>Vylepšené rozpoznávání typu disku</li>
<li>Informační okno s podrobnostmi o vloženém disku</li>
<li>Vylepšený a rozšířený dialog nastavení</li>
<li>Přepracovaná a vylepšená dokumentace</li>
<li>Ruský překlad od Igor Gorbounov</li>
<li>... a spousta malých změn a oprav.</li>
</ul><b>Opravy</b> (malé změny po verzi 0.72; výše uvedené soubory byly aktualizovány):<p>
<b>0.72 pl4</b> Updated for new versions and programming libraries of
GNU/Linux, FreeBSD and NetBSD. (07-Apr-2012)<p>

<b>0.72 pl3</b> Opraven problém s funkcí &quot;Ověřit&quot; při práci se soubory
pro opravu chyb RS01 většími než 2GB. Poděkování za nahlášení chyby a zaslání
opravy patří Volodymyru Bychkoviakovi. (2011-10-05)<p>

<b>0.72 pl2</b> Tato verze přináší alternativní řešení chyby kvůli které v Linuxu docházelo k zamrzání paralelních SCSI adaptérů. Byla vylepšena kompatibilita s verzemi 0.79.x.<br> Verze pro Windows a Mac OS X jsou nyní sestavovány ve vývojovém prostředí pro dvdisaster 0.79.x a jsou tak dodávány s novějšími verzemi knihoven grafického sady nástrojů GTK+. Tato aktualizace si vyžádala určité změny v interních skriptech, takže došlo ke změně kontrolního součtu balíčku zdrojových kódů (balíček vydaný 31. října měl kontrolní součet md5 86110e212aa1bf336a52ba89d3daa93d a je stále platný pro Linux, FreeBSD a NetBSD).(2010-07-11)<p><b>0.72 pl1</b> Pablo Almeida vytvořil portugalský překlad programu. Přidána funkce která má předejít zamrzání Win XP u některých kombinací CD-RW/mechanik.(2009-08-08)<p><i>Aktualizace: tato funkce nefunguje vždy. Do <a href="download40.php">verze 0.79.x</a> byla přidána vylepšená verze, bohužel však nemůže být jednoduše použita ve stabilní verzi.</i> (06-Feb-2010)<p><b>0.72</b> První stabilní verze větve 0.72. Igor Gorbounov dokončil ruskou online dokumentaci. Byly opraveny některé chyby nalezené v první RC verzi.<p>Novější verze Windows nemusí při užití určitých jazykových nastavení použít správný jazyk zobrazení. Jde o složitější problém a bude proto vyřešen až v některé z verzí 0.73.x. (2009-07-04)<p><b>0.72-rc1</b> První RC. (2009-04-11)</td></tr></table><p>Pokud se vám nepodaří stáhnout dvdisaster pomocí výše uvedených odkazů, zkuste ho získat přímo na stránkách <a href="http://sourceforge.net/projects/dvdisaster/files">SourceForge</a>.<pre> </pre><b>Předešlá verze</b> - aktualizace na verzi 0.72 doporučena.<p><table class="download" cellpadding="0" cellspacing="5">
<tr><td><b>dvdisaster-0.70</b></td><td align="right">2008-03-04</td></tr>
<tr><td colspan="2" class="hsep"></td></tr>
<tr><td colspan="2"><table>
    <tr><td align="right">  Zdrojový kód (všechny operační systémy): </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.70.6.tar.bz2">dvdisaster-0.70.6.tar.bz2</a></td></tr>
    <tr><td align="right">Digitální podpis </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.70.6.tar.bz2.gpg">dvdisaster-0.70.6.tar.bz2.gpg</a></td></tr>
    <tr><td align="right">Kontrolní součet MD5: </td>
        <td>c6d2215d7dd582475b19593dfa4fbdc2</td></tr>
    <tr><td colspan="2" class="esep"></td></tr>

    <tr><td align="right">Binární soubor pro Windows: </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.70.6-setup.exe">dvdisaster-0.70.6-setup.exe</a></td></tr>
    <tr><td align="right">Digitální podpis </td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.70.6-setup.exe.gpg">dvdisaster-0.70.6-setup.exe.gpg</a></td></tr>
    <tr><td align="right">Kontrolní součet MD5: </td>
        <td>82f74bebd08ab7ae783ddc5dd0bba731</td></tr>
  </table>
</td></tr>
<tr><td colspan="2" class="hsep"></td></tr>
<tr><td colspan="2">Metoda opravy chyb RS02 je plně podporována v grafickém uživatelském rozhraní. Bitové kopie vytvořené metodou RS02 mohou být použity s využitím adaptivní strategie čtení.<p>Julian Einwag začal pracovat na portu dvdisaster pro Mac OS X / Darwin.<p>Daniel Nylander vytvořil švédský překlad programu.<p><b>Opravy</b> (malé změny po verzi 0.70; výše uvedené soubory byly aktualizovány):<p><b>pl6</b> Odstraněna podpora pro lokalizované názvy souborů, protože způsobovala problémy s podporou velkých souborů ve Windows. Nová obslužná rutina pro lokalizované názvy souborů bude otestována ve zkušební verzi 0.71.25. <i>(2008-03-04)</i><p><b>pl5</b> Opraven problém ke kterému docházelo při použití novějších linuxových jader a který za určitých okolností mohl vést k zamrznutí systému. Vylepšena práce s názvy souborů obsahujícími lokalizované znaky. Obsahuje několik důležitějších oprav převzatých z testovací verze 0.71.24. <i>(2008-02-24)</i>.<p><b>pl4</b> Poskytuje vylepšenou kompatibilitu s dvouvrstvými DVD (DVD-R DL a DVD+R DL).<br>Byly opraveny některé menší chyby. <i>(2007-01-20)</i>.<p><b>pl3</b> Opravuje chybné rozpoznávání nepodporovaných formátů CD které ve výjimečných případech vedlo k pádu Windows (BSOD). Přidána možnost přerušení inicializace RS02 na DVD RW médiu. <i>(2006-12-10)</i>.<p><b>pl2</b> Opravuje chybné uvolnění paměti po uzavření okna aplikace. Opraveno rozbalení snímků obrazovky dokumentace na PPC platformě. Byly aktualizovány pouze balíčky se zdrojovými kódy. <i>(2006-10-03)</i>.<p><b>pl1</b> Opravuje chybu u adaptivního čtení pro RS02 díky které v některých případech nebyl načten dostatek dat pro úspěšnou obnovu dat. Přidáno několik malých vylepšení dokumentace a použitelnosti. <i>(2006-07-30)</i></td></tr></table><p>Zdrojový kód dvdisaster je <a href="http://sourceforge.net/cvs/?group_id=157550">dostupný pomocí CVS</a>. Některé další soubory s důležitými informacemi:<ul>
<li><a href="http://dvdisaster.cvs.sourceforge.net/dvdisaster/dvdisaster/CHANGELOG?view=markup">CHANGELOG</a>- seznam změn mezi jednotlivými verzemi;</li>
<li><a href="http://dvdisaster.cvs.sourceforge.net/dvdisaster/dvdisaster/CREDITS.en?view=markup">CREDITS.en</a>- seznam lidí podílejících se na tomto projektu;</li>
<li><a href="http://dvdisaster.cvs.sourceforge.net/dvdisaster/dvdisaster/INSTALL?view=markup">INSTALL</a> - dodatečné tipy pro instalaci;</li>
<li><a href="http://dvdisaster.cvs.sourceforge.net/dvdisaster/dvdisaster/README?view=markup">README</a> - obecný návod k balíčkům zdrojového kódu.</li>
</ul>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
