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
?>

<!-- Insert actual page content below -->

<h3 class="top">Восстановление образов носителей</h3>

<table width="100%" cellspacing="5">
<tr valign="top">
<td class="w20p"><b>Задача</b></td>
<td>
Восстановить содержимое поврежденного носителя.
</td>
</tr>
<tr><td> <pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Требуется:</b><p></td>
</tr>
<tr>
 <td class="w150x" align="right">
   <img src="../images/bad-cd-ecc.png" alt="Icon: Damaged medium with error correction data" class="valignt">
 </td>
<td>
Поврежденный носитель, содержащий <a href="howtos30.php">данные для исправления ошибок</a>,
</td>
</tr>
<tr><td></td><td>или</td></tr>
<tr>
 <td class="w150x" align="right">
   <img src="../images/bad-cd.png" alt="Icon: Damaged medium (partially unreadable)">
   <img src="../images/ecc.png" alt="Icon: Separate file with error correction data">
 </td>
<td>
поврежденный носитель с соответствующим <a href="howtos20.php">файлом для исправления ошибок</a><a href="#footnote"><sup>*)</sup></a>.
</td>
</tr>
<tr><td> <pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Что нужно сделать:</b><p></td>
</tr>

<tr>
<td></td>
<td>
1. <a href="howtos41.php">Настроить основные параметры для чтения,</a><br>
2a. <a href="howtos42.php#a">создать ISO-образ из поврежденного носителя,</a><br>
2b. <a href="howtos42.php#b">восстановить образ и записать его на новый носитель.</a>
</td>
</tr>
</table><p>

<a href="howtos42.php">Создание и восстановление ISO-образа...</a>

<pre>


</pre>


<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
