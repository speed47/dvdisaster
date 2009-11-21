<?php
# dvdisaster: Russian homepage translation
# Copyright (C) 2007-2009 Igor Gorbounov
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

require("../include/dvdisaster.php");
begin_page();
?>

<!--- Insert actual page content below --->

<h3>Альфа-версии (для разработчиков)</h3>

<b>Помогите нам с тестированием!</b> На этой странице находятся экспериментальные версии dvdisaster,
создаваемые на пути к следующему стабильному выпуску.<p>

<b>Предупреждение:</b> Альфа-версии не прошли тщательного тестирования. В них может
быть больше ошибок, чем в стабильной версии, и их не следует использовать
для обработки важных данных.<p>

Если есть сомнения, то продолжайте использовать <a href="download.php">стабильную версию 0.72</a>
и ждите выпуска версии 0.74.

<hr>

<h3>Загрузки</h3>

Для альфа-версий используется такой же формат пакетов, как и для нормальных версий.<p>

<table class="download" cellpadding="0" cellspacing="5">
<tr><td><b>dvdisaster-0.73 (devel1)</b></td><td align="right">xx-XXX-2009</td></tr>
<tr bgcolor="#000000"><td colspan="2"><img width=1 height=1 alt=""></td></tr>
<tr><td colspan="2">
  <table>
    <tr><td align="right">&nbsp;&nbsp;Исходные тексты для всех операционных систем:&nbsp;</td>
        <td><a href="http://prdownloads.sourceforge.net/dvdisaster/dvdisaster-0.73.1.tar.bz2?download">dvdisaster-0.73.1.tar.bz2</a></td></tr>
    <tr><td align="right">Цифровая подпись:&nbsp;</td>
        <td><a href="http://prdownloads.sourceforge.net/dvdisaster/dvdisaster-0.73.1.tar.bz2.gpg?download">dvdisaster-0.73.1.tar.bz2.gpg</a></td></tr>
    <tr><td align="right">Двоичная версия для Windows:&nbsp;</td>
        <td><a href="http://prdownloads.sourceforge.net/dvdisaster/dvdisaster-0.73.1-setup.exe?download">dvdisaster-0.73.1-setup.exe</a></td></tr>
    <tr><td align="right">Цифровая подпись:&nbsp;</td>
        <td><a href="http://prdownloads.sourceforge.net/dvdisaster/dvdisaster-0.73.1-setup.exe.gpg?download">dvdisaster-0.73.1-setup.exe.gpg</a></td></tr>
  </table>
</td></tr>
<tr bgcolor="#000000"><td colspan="2"><img width=1 height=1 alt=""></td></tr>
<tr><td colspan="2">
Еще не выпущена.
</td></tr></table><p>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
