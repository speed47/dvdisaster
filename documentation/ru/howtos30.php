<?php
# dvdisaster: English homepage translation
# Copyright (C) 2004-2009 Carsten Gnörlich
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

require("../include/dvdisaster.php");
require("../include/footnote.php");
begin_page();
howto_headline("Дополнение образов данными для исправления ошибок", "Обзор", "images/create-icon.png");
?>

<!--- Insert actual page content below --->

<h3>Помещение данных для исправления ошибок непосредственно на носитель</h3>

<table width="100%" cellspacing="5">
<tr valign="top">
<td width="20%"><b>Задача</b></td>
<td>
Данные для исправления ошибок сохраняются вместе с пользовательскими данными на том же носителе.
</td>
</tr>
<tr><td> <pre> </pre> </td></tr>

<tr valign="top">
<td></td>
<td>Примечание: на этой странице описывается, как ISO-образ дополняется данными для
исправления ошибок перед его записью на носитель.
Существует также метод создания и размещения данных для исправления ошибок в
отдельном файле. 
<a href="howtos21.php">Вам требуется помощь в выборе между этими двумя методами?</a></td>
<tr><td> <pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Требуется:</b><p></td>
</tr>

<tr>
<td><img src="../images/good-image.png"></td>
<td>
<ul>
<li>программа для создания ("прожига") дисков, способная создавать ISO-образы</li>
<li>носитель, на который должны быть добавлены данные для исправления ошибок, 
еще не записанный <a href="#footnote"><sup>*)</sup></a></li>
<li>не менее 20% свободного места на носителе, который будет создаваться</li>
</ul>
</tr>
<tr><td> <pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Что делать:<p></b></td>
</tr>

<tr>
<td></td>
<td>
1. <a href="howtos32.php">Настроить основные параметры</a><p>
2a. <a href="howtos33.php#a">Создать ISO-образ,</a><br>
2b. <a href="howtos33.php#b">дополнить его данными для исправления ошибок</a><br>
2c. <a href="howtos33.php#c">и записать его на носитель.</a>
</td>
</tr>
</table><p>

<a href="howtos32.php">Настройка основных параметров...</a>

<pre>


</pre>


<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
