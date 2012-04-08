<?php
# dvdisaster: Russian homepage translation
# Copyright (C) 2007-2012 Igor Gorbounov
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

require("../include/dvdisaster.php");
begin_page();

howto_headline("Поиск ошибок на носителях", "Обзор", "images/scan-icon.png");
?>

<!-- Insert actual page content below -->

<table width="100%" cellspacing="5">
<tr>
 <td><b>Задача</b></td>
 <td>
   Носитель проверяется на нечитаемые сектора.
  </td>
</tr>
<tr><td> <pre> </pre> </td></tr>

<tr>
 <td colspan="2"><b>Требуется:</b></td>
</tr>
<tr>
 <td width="150px"><img src="../images/good-cd.png" alt="Icon: Good medium (without read errors)" class="valignt">
   &nbsp; <img src="../images/bad-cd.png" alt="Icon: Damaged medium (partially unreadable)" class="valignt"></td>
<td>
   Носитель в любом состоянии (хорошем или с ошибками чтения).
</td>
</tr>

<tr>
 <td><img src="../images/ecc.png" alt="Icon: Separate file with error correction data"></td>
 <td>Если имеются данные для исправления ошибок, выполняются дополнительные тесты.
Но поиск ошибок работает и без данных для исправления ошибок.</td>
</tr>

<tr><td> <pre> </pre> </td></tr>

<tr valign="top">
 <td><b>Что делать:</b></td>
 <td>
  1. <a href="howtos11.php">Конфигурирование основных настроек</a><br>
  2. <a href="howtos12.php">Проверка носителя</a><br>
  3. <a href="howtos13.php">Интерпретация результатов</a><br>
 </td>
</tr>

<tr><td> <pre> </pre> </td></tr>

<tr valign="top">
 <td><b>Соответствующие функции:</b></td>
 <td><a href="howtos30.php">Чтение поврежденных носителей</a> и<br>
     <a href="howtos40.php">Восстановление образов</a>.</td>
 </tr>
</table><p>

<pre> </pre>

<a href="howtos11.php">Конфигурирование основных настроек...</a>


<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
