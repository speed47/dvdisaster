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
howto_headline("Создание данных для исправления ошибок в отдельном файле", "Обзор", "images/create-icon.png");?>

<!-- Insert actual page content below -->

<table width="100%" cellspacing="5">
<tr valign="top">
<td class="w20p"><b>Задача</b></td>
<td>
Создается файл данных для исправления ошибок для носителя CD/DVD/BD.
</td>
</tr>
<tr><td> <pre> </pre> </td></tr>

<tr valign="top">
<td></td>
<td>Примечание: На этой странице описывается, как создаются и помещаются
в отдельный файл данные для исправления ошибок. 
Есть также метод для размещения данных для исправления ошибок
непосредственно на носитель.
<a href="howtos21.php">Вам нужна помощь, чтобы сделать выбор между этими двумя методами?</a></td>
<tr><td> <pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Требуется:</b><p></td>
</tr>

<tr>
<td><img src="../images/good-cd.png" alt="Icon: Good medium (without read errors)"></td>
<td>
Хороший, без ошибок, <a href="#footnote"><sup>*)</sup></a> носитель,</td>
</tr>

<tr><td></td><td>или</td></tr>


<tr>
<td><img src="../images/good-image.png" alt="Icon: Complete image"></td>
<td>уже существующий готовый<a href="#footnote"><sup>*)</sup></a> 
ISO-образ носителя (например, образ, используемый для записи носителя).
</td>
</tr>
<tr><td> <pre> </pre> </td></tr>


<tr>
<td colspan="2"><b>Что делать:</b><p></td>
</tr>

<tr>
<td></td>
<td>
1. <a href="howtos22.php">Настроить основные параметры</a><br>
2. <a href="howtos23.php">Создать файл для исправления ошибок</a><br>
3. <a href="howtos24.php">Архивировать файл для исправления ошибок</a>
</td>
</tr>
</table><p>

<a href="howtos22.php">Настройка основных параметров...</a>

<pre>


</pre>

<?php
footnote("*","footnote","Данные для исправления ошибок должны быть созданы прежде, чем
произойдет потеря данных: невозможно создавать файлы для исправления ошибок 
с уже поврежденного носителя.");
?>


<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
