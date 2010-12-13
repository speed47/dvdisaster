<?php
# dvdisaster: Russian homepage translation
# Copyright (C) 2007-2010 Igor Gorbounov
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

<!--- Insert actual page content below --->

<h3>Скачать dvdisaster</h3>

dvdisaster имеется для <a href="download10.php">последних версий</a>
операционных систем FreeBSD, GNU/Linux, Mac OS X(Darwin), NetBSD 
и Windows. Он предоставляется как
<a href="http://www.germany.fsfeurope.org/documents/freesoftware.en.html">свободное программное обеспечение</a> 
под лицензией <a href="http://dvdisaster.cvs.sourceforge.net/dvdisaster/dvdisaster/COPYING?view=markup">GNU General Public License v2</a><a href="#gpl3"><sup>*)</sup></a>.<p>

Скачивать исходные тексты или двоичную версию можно из приведенного ниже списка.
<a href="download20.php">Цифровая подпись</a> предоставляется для подтверждения того, что
пакеты находятся в своем первоначальном состоянии.<p>

<ul>
<li>В пакете с исходными текстами имеется файл <tt>INSTALL</tt>, содержащий дальнейшие инструкции для сборки.</li>
<li>Для Mac OS X предоставлен ZIP-архив, который устанавливается путем его распаковывания
в любое место. Обратите внимание на 
<a href="download30.php#mac">специальные советы для Mac OS X</a>.</li>
<li>Для <a href="download30.php#win">установки</a> двоичной версии для Windows
запустите загруженную программу на выполнение и действуйте в соответствии с диалогом.</li>
</ul>

<?php
if(!strcmp($have_experimental, "yes"))
{ ?>
<b>Альфа (нестабильные) версии</b> - новые и экспериментальные для опытных пользователей!<p>

Приглашаем тестеров для предстоящих версий dvdisaster, но следует иметь в виду, что
остались еще ошибки и несоответствия.
Текущая нестабильная версия - 
<a href="download40.php"><?php echo ${cooked_version}?></a>.
<p>

<?php
}
?>

<b>Стабильная версия</b> - рекомендуется для начала.<p>
<a name="download"></a>

<table class="download" cellpadding="0" cellspacing="5">
<tr><td><b>dvdisaster-0.72</b></td><td align="right">07 Nov 2010</td></tr>
<tr bgcolor="#000000"><td colspan="2"><img width=1 height=1 alt=""></td></tr>
<tr><td colspan="2">
  <table>
    <tr><td align="right">&nbsp;&nbsp;Исходные тексты для всех операционных систем:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.2.tar.bz2">dvdisaster-0.72.2.tar.bz2</a></td></tr>
    <tr><td align="right">Цифровая подпись:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.2.tar.bz2.gpg">dvdisaster-0.72.2.tar.bz2.gpg</a></td></tr>

<?php
if($mode == "www");
    echo "<tr><td align=\"right\">MD5 checksum:&nbsp;</td><td>312bceef3bf9c0754cf633ed3b12eb71</td></tr>";
?>
    <tr><td colspan="2"><img width=1 height=3</td></tr>

    <tr><td align="right">Двоичная версия для Mac OS X 10.5 / x86:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.2.app.zip">dvdisaster-0.72.2.app.zip</a>&nbsp;--&nbsp;сначала прочитайте эти <a href="download30.php#mac">советы</a></td></tr>
    <tr><td align="right">Цифровая подпись:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.2.app.zip.gpg">dvdisaster-0.72.2.app.zip.gpg</a></td></tr>

<?php
if($mode == "www");
    echo "<tr><td align=\"right\">MD5 checksum:&nbsp;</td><td>52243c1fafb9d2e496b6eb318c3e534f</td></tr>";
?>
    <tr><td colspan="2"><img width=1 height=3</td></tr>

    <tr><td align="right">Двоичная версия для Windows:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.2-setup.exe">dvdisaster-0.72.2-setup.exe</a></td></tr>
    <tr><td align="right">Цифровая подпись:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.2-setup.exe.gpg">dvdisaster-0.72.2-setup.exe.gpg</a></td></tr>
<?php
if($mode == "www");
    echo "<tr><td align=\"right\">MD5 checksum:&nbsp;</td><td>f80258d27354061fd9e28850ec4701a6</td></tr>";
?>
    <tr><td colspan="2"> </td></tr>

<?php
  if($show_all == 0) {
?>
    <tr><td colspan="2"><a href="download.php?showall=1#download">Show older releases in the 0.72 version branch</a></td></tr>
<?php
  }
  else {
?> 
    <tr><td colspan="2"><a href="download.php?showall=0#download">Hide older releases in the 0.72 version branch</a></td></tr>

   <tr><td colspan="2"> </td></tr>
   <tr><td></td><td>Version 0.72.1</td></tr>
    <tr><td align="right">&nbsp;&nbsp;Исходные тексты для всех операционных систем:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.1.tar.bz2">dvdisaster-0.72.1.tar.bz2</a></td></tr>
    <tr><td align="right">Цифровая подпись:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.1.tar.bz2.gpg">dvdisaster-0.72.1.tar.bz2.gpg</a></td></tr>
    <tr><td align="right">MD5 checksum:&nbsp;</td>
        <td>4da96566bc003be93d9dfb0109b4aa1d</td></tr>
    <tr><td colspan="2"><img width=1 height=3</td></tr>

    <tr><td align="right">Двоичная версия для Mac OS X 10.5 / x86:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.1.app.zip">dvdisaster-0.72.1.app.zip</a>&nbsp;--&nbsp;сначала прочитайте эти <a href="download30.php#mac">советы</a></td></tr>
    <tr><td align="right">Цифровая подпись:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.1.app.zip.gpg">dvdisaster-0.72.1.app.zip.gpg</a></td></tr>
    <tr><td align="right">MD5 checksum:&nbsp;</td>
        <td>924b5677f69473b6b87991e01779a541</td></tr>
    <tr><td colspan="2"><img width=1 height=3</td></tr>

    <tr><td align="right">Двоичная версия для Windows:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.1-setup.exe">dvdisaster-0.72.1-setup.exe</a></td></tr>
    <tr><td align="right">Цифровая подпись:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.1-setup.exe.gpg">dvdisaster-0.72.1-setup.exe.gpg</a></td></tr>
    <tr><td align="right">MD5 checksum:&nbsp;</td>
        <td>34d062ddebe1a648e808d29ca4e9879f</td></tr>

   <tr><td colspan="2"> </td></tr>
   <tr><td></td><td>Version 0.72</td></tr>
    <tr><td align="right">&nbsp;&nbsp;Исходные тексты для всех операционных систем:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.tar.bz2">dvdisaster-0.72.tar.bz2</a></td></tr>
    <tr><td align="right">Цифровая подпись:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.tar.bz2.gpg">dvdisaster-0.72.tar.bz2.gpg</a></td></tr>
    <tr><td align="right">MD5 checksum:&nbsp;</td>
        <td>efa35607d91412a7ff185722f270fb8a</td></tr>
    <tr><td colspan="2"><img width=1 height=3</td></tr>

    <tr><td align="right">Двоичная версия для Mac OS X 10.5 / x86:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.app.zip">dvdisaster-0.72.app.zip</a>&nbsp;--&nbsp;сначала прочитайте эти <a href="download30.php#mac">советы</a></td></tr>
    <tr><td align="right">Цифровая подпись:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.app.zip.gpg">dvdisaster-0.72.app.zip.gpg</a></td></tr>
    <tr><td align="right">MD5 checksum:&nbsp;</td>
        <td>1f28385b2b6d64b664fd416eb4c85e80</td></tr>
    <tr><td colspan="2"><img width=1 height=3</td></tr>

    <tr><td align="right">Двоичная версия для Windows:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72-setup.exe">dvdisaster-0.72-setup.exe</a></td></tr>
    <tr><td align="right">Цифровая подпись:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72-setup.exe.gpg">dvdisaster-0.72-setup.exe.gpg</a></td></tr>
    <tr><td align="right">MD5 checksum:&nbsp;</td>
        <td>cc8eb2af384917db8d6d983e1d4aac69</td></tr>

<?php
  }
?>
  </table>
</td></tr>
<tr bgcolor="#000000"><td colspan="2"><img width=1 height=1 alt=""></td></tr>
<tr><td colspan="2">
Наиболее важные изменения в этой версии:<p>
<ul>
<li>Поддержка <a href="qa10.php#media">носителей Blu-Ray</a></li>
<li>Низкоуровневое чтение и проверки на C2 для CD-носителей</li>
<li>Выбираемое число попыток чтения</li>
<li>Первый "родной" пакет приложения для Mac OS X</li>
<li>Порт на NetBSD Сергея Свищева</li>
<li>Улучшенное распознавание типа носителей</li>
<li>Информационное окно с описанием вставленного носителя</li>
<li>Улучшенный и дополненный диалог настроек</li>
<li>Переработанная и дополненная документация</li>
<li>Русский перевод Игоря Горбунова</li>
<li>... и еще много небольших изменений и исправлений.</li>
</ul>

<b>Исправления</b> (небольшие изменения после версии 0.72; вышеприведенные файлы были обновлены):<p>

<b>0.72 pl2</b> 
This version introduces a workaround which prevents parallel SCSI
adapters from freezing under Linux. 
Upward compatibility with versions 0.79.x has been improved. <br>
The Windows and Mac OS X versions are now built with the development
environment of dvdisaster 0.79.x and are therefore shipped with newer
versions of the GTK+ graphical toolkit libraries. This update requires
some changes in internal scripts resulting in a different checksum of the
source package (the package published on Oct 31th had the md5 checksum
86110e212aa1bf336a52ba89d3daa93d and is still valid for Linux, FreeBSD 
and NetBSD).(07-11-2010)<p>

<b>0.72 pl1</b> Pablo Almeida provided Portuguese translations of the screen texts.
Added workaround to avoid Win XP freezing on certain CD-RW/drive pairs.(08-Aug-2009)<p>
<i>Update: The workaround has been found to be ineffective in some cases. A
better workaround is included in <a href="download40.php">version 0.79.x</a>;
unfortunately it can not be easily backported into the stable version.</i> (06-Feb-2010)<p>

<b>0.72</b> Это первая стабильная версия ветки 0.72.
Игорь Горбунов завершил перевод онлайн-документации на русский язык.
Устранены некоторые незначительные ошибки в первом кандидате на выпуск.<p>
В более новых версиях Windows при некоторых языковых настройках может быть неправильный вывод
на экран. Это довольно сложная проблема и она будет решаться
в предстоящих версиях 0.73.x. (04-Jul-2009)
<p>

<b>0.72-rc1</b> Первый кандидат на выпуск. (11 апреля 2009)
</td></tr></table><p>

If the links above fail to download 
please try getting dvdisaster via
<a href="http://sourceforge.net/projects/dvdisaster/files">SourceForge</a>.

<pre> </pre>

<b>Предыдущая версия</b> - рекомендуется обновить до версии 0.72.<p>

<table class="download" cellpadding="0" cellspacing="5">
<tr><td><b>dvdisaster-0.70</b></td><td align="right">04 марта 2008</td></tr>
<tr bgcolor="#000000"><td colspan="2"><img width=1 height=1 alt=""></td></tr>
<tr><td colspan="2">
  <table>
    <tr><td align="right">&nbsp;&nbsp;Исходный текст для всех операционных систем:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.70.6.tar.bz2">dvdisaster-0.70.6.tar.bz2</a></td></tr>
    <tr><td align="right">Цифровая подпись:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.70.6.tar.bz2.gpg">dvdisaster-0.70.6.tar.bz2.gpg</a></td></tr>
    <tr><td align="right">MD5 checksum:&nbsp;</td>
        <td>c6d2215d7dd582475b19593dfa4fbdc2</td></tr>
    <tr><td colspan="2"><img width=1 height=3</td></tr>

    <tr><td align="right">Двоичный пакет для Windows:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.70.6-setup.exe">dvdisaster-0.70.6-setup.exe</a></td></tr>
    <tr><td align="right">Цифровая подпись:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.70.6-setup.exe.gpg">dvdisaster-0.70.6-setup.exe.gpg</a></td></tr>
    <tr><td align="right">MD5 checksum:&nbsp;</td>
        <td>82f74bebd08ab7ae783ddc5dd0bba731</td></tr>

  </table>
</td></tr>
<tr bgcolor="#000000"><td colspan="2"><img width=1 height=1 alt=""></td></tr>
<tr><td colspan="2">
Метод коррекции ошибок RS02
полностью поддерживается в графическом интерфейсе. Образы, созданные
с использованием RS02, могут использоваться
со стратегией адаптивного чтения.<p>

Джулиан Айнваг (Julian Einwag) начал перенос dvdisaster 
на Mac OS X / Darwin.<p>

Дэниэл Найландер (Daniel Nylander) дал шведский перевод экранных текстов.<p>

<b>Исправления</b> (небольшие изменения после версии 0.70; вышеприведенные файлы были обновлены):<p>

<b>pl6</b> Сделан откат назад поддержки локализованных имен файлов, 
поскольку она нарушила поддержку больших файлов под Windows. Новый обработчик
для локализованных имен файлов будет сначала протестирован в экспериментальной версии
0.71.25. <i>(04-мар-2008)</i><p>

<b>pl5</b> 
Исправляет проблему с новыми ядрами Linux, которая может привести к зависанию системы в
некоторых случаях. Улучшена обработка имен файлов, содержащих локализованные
символы. 
Содержит обратный перенос основных исправлений ошибок с 0.71.24. <i>(24-фев-2008)</i>.<p>

<b>pl4</b> обеспечивает лучшую совместимость с двухслойными DVD
(DVD-R DL и DVD+R DL).<br> 
Были исправлены некоторые незначительные ошибки. <i>(20-янв-2007)</i>.<p>

<b>pl3</b> исправляет неправильное распознавание неподдерживаемых форматов CD, которые
приводили к синему экрану Windows при редком стечении обстоятельств. Добавлена возможность отмены
во время инициализации RS02 на носителях DVD RW.
<i>(10-дек-2006)</i>.<p>

<b>pl2</b> исправляет неправильное освобождение памяти, когда закрывается окно программы.
Исправлена распаковка снимков экрана в документации для платформ PPC.
Обновлены только архивы исходных текстов.
<i>(03-окт-2006)</i>.<p>

<b>pl1</b> исправляет ошибку в адаптивном чтении для RS02, которая иногда приводит к
чтению недосточного количества данных при сообщении об успешном восстановлении. Добавляет несколько
небольших улучшений в документации и функциональности. <i>(30-июл-2006)</i>
</td></tr></table><p>


Исходные тексты dvdisaster доступны также
<a href="http://sourceforge.net/cvs/?group_id=157550">через CVS</a>.
Файлы, представляющие интерес:
<ul>
<li><a href="http://dvdisaster.cvs.sourceforge.net/dvdisaster/dvdisaster/CHANGELOG?view=markup">CHANGELOG</a>- изменения от предыдущих версий;</li>
<li><a href="http://dvdisaster.cvs.sourceforge.net/dvdisaster/dvdisaster/CREDITS.en?view=markup">CREDITS.en</a>- люди, занятые в проекте;</li>
<li><a href="http://dvdisaster.cvs.sourceforge.net/dvdisaster/dvdisaster/INSTALL?view=markup">INSTALL</a> - дополнительные советы для установки;</li>
<li><a href="http://dvdisaster.cvs.sourceforge.net/dvdisaster/dvdisaster/README?view=markup">README</a> - план развития из архива исходных текстов.</li>
</ul>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
