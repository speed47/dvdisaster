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
begin_page();
?>

<!--- Insert actual page content below --->

<h3>Неправильное использование dvdisaster</h3>

Джо поспорил о том, что его носители сохранят свое содержимое
без дополнительной защиты.<p>

<table width="100%">
<tr>
<td width="15%">10 фев. 2004</td>
<td width="60px"><img src="../images/good-cd.png"></td>
<td width="60px"><img src="../images/good-cd.png"></td>
<td>Джо создает два CD с важными данными. Но он не предпринимает
никаких мер предосторожности против потери данных на этих носителях.</td>
</tr>
<tr><td colspan="4"> <hr> </td></tr>
<tr>
<td>14 мая 2005</td>
<td><img align="top" src="../images/good-cd.png"></td>
<td><img align="top" src="../images/good-cd.png"></td>
<td>Джо регулярно пользуется своими CD. Спустя год они по-прежнему 
прекрасно читаются.</td>
</tr>
<tr><td colspan="4"> <hr> </td></tr>
<tr>
<td>19 авг. 2007</td>
<td><img align="top" src="../images/bad-cd.png"></td>
<td><img align="top" src="../images/good-cd.png"></td>
<td>Еще через два года Джо замечает, что некоторые данные на одном CD больше
не читаются.</td> 
</tr>
<tr>
 <td align="right"><a href="howtos10.php">поиск</a></td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td></td>
</tr>
<tr>
<td>20 авг. 2007</td>
<td><img align="top" src="../images/bad-cd.png"></td>
<td><img align="top" src="../images/bad-cd.png"></td>
<td>Джо загружает dvdisaster и выполняет
<a href="howtos10.php">поиск ошибок чтения</a>. 
Он обнаруживает, что на этом CD 25000 нечитаемых секторов. Проверка
второго CD показывает, что на нем образовалось 1500 нечитаемых секторов,
не замечавшихся до сих пор. </td>
</tr>
<tr>
 <td align="right"><a href="howtos30.php">чтение</a></td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td></td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td>21 авг. 2007</td>
<td><img align="top" src="../images/bad-image.png"></td>
<td><img align="top" src="../images/bad-image.png"></td>
<td>Джо использует dvdisaster, чтобы
<a href="howtos30.php">считать как можно больше секторов</a> с поврежденного 
носителя. Но поскольку у него нет данных для исправления ошибок, 
нет способа повторного расчета нечитаемых секторов.</td>
</tr>
<tr>
 <td align="right"><a href="howtos30.php">много попыток<br>чтения</a></td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td></td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td>05 сен. 2007</td>
<td><img align="top" src="../images/bad-image.png"></td>
<td><img align="top" src="../images/good-image.png"></td>
<td>Джо использует способность dvdisaster'а делать образы полными 
с помощью многократных проходов чтения. Он переносит дефектные образы 
на разные компьютеры, чтобы делать попытки чтения на разных дисководах.
Через две недели попыток по крайней мере на втором CD все недостающие
секторы были прочитаны. Однако на первом CD 21000 секторов все же
остались нечитаемыми всеми дисководами, которые он попробовал.</td>
</tr>
<tr>
 <td align="right">только один CD<br>восстановлен</td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td></td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td>06 сен. 2007</td>
<td><img align="top" src="../images/bad-cd.png"></td>
<td><img align="top" src="../images/good-cd.png"></td>
<td>Джо выбрасывает первый CD, посчитав его невосстановимым, и думает, что ему
повезло, что он получил полный образ второго CD и может записать его на новый носитель.
Но если бы он своевременно создал данные для исправления ошибок, то, возможно<sup>1)</sup>,
ему бы не пришлось тратить две недели на попытки прочитать диск и он бы восстановил
содержимое обоих CD.</td></tr>
</table>
<hr>
<sup>1)</sup>Механизм исправления ошибок ориентируется на типичный процесс старения.
Если CD оказывается сильно поврежденным, он становится невосстановимым даже с
помощью данных для исправления ошибок. Не надейтесь только на dvdisaster при защите
важных данных; принимайте дополнительные меры, например, создание  
дополнительных копий на разных типах носителей.

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
