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

<h3>Правильное использование dvdisaster</h3>

Покажем, как Джейн использует dvdisaster. <p>

<table width="100%">
<tr>
<td width="15%">10 фев. 2004</td>
<td width="60px"><img src="../images/good-cd.png"></td>
<td width="60px"></td>
<td>Джейн создает новый CD с важными данными.</td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td> </td>
<td><img align="top" src="../images/good-cd.png"></td>
<td><img align="top" src="../images/ecc.png"></td>
<td>Чтобы защитить CD от потери данных,
    <a href="howtos20.php">она создает данные для исправления ошибок с помощью dvdisaster</a>.
    Она хранит оба вида данных для последующего использования.</td>
</tr>
<tr><td colspan="4"> <hr> </td></tr>
<tr>
<td>14 мая 2005</td>
<td><img align="top" src="../images/good-cd.png"></td>
<td><img align="top" src="../images/ecc.png"></td>
<td> Джейн знает, что при ежедневном использовании, возможно, не ко всем данным на ее CD  
осуществляется доступ. Поэтому после того, как прошел год,
она <a href="howtos10.php">проверяет CD на ошибки чтения</a>, чтобы быть
уверенной, что никакие дефекты не появились в редко используемых местах. Однако
после одного года CD все еще отлично читается.</td>
</tr>
<tr><td colspan="4"> <hr> </td></tr>
<tr>
<td>19 авг. 2007</td>
<td><img align="top" src="../images/bad-cd.png"></td>
<td><img align="top" src="../images/ecc.png"></td>
<td>Прошло еще два года и Джейн замечает, что некоторые данные на 
CD больше не читаются. <a href="howtos10.php">Проверка на ошибки чтения</a> 
подтверждает, что на CD появились дефекты вследствие старения.</td>
</tr>
<tr>
 <td align="right"><a href="howtos30.php">чтение</a></td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td></td><td></td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td> </td>
<td><img align="top" src="../images/bad-image.png"></td>
<td><img align="top" src="../images/ecc.png"></td>
<td>Джейн использует dvdisaster, чтобы <a href="howtos30.php">прочитать как можно больше
секторов</a> с поврежденного CD в ISO-образ.</td>
<tr>
 <td align="right"><a href="howtos40.php">воссоздание</a></td>
 <td align="center" colspan="2"><img align="top" src="../images/dbl-arrow-left.png"></td>
 <td></td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td> </td>
<td><img align="top" src="../images/good-image.png"></td>
<td><img align="top" src="../images/ecc.png"></td>
<td>Используя данные для исправления ошибок, Джейн
    <a href="howtos40.php">восстанавливает недостающие части ISO-образа</a>.
<tr>
 <td align="right">Запись нового CD</td>
 <td align="center"><img align="top" src="../images/down-arrow.png"></td>
 <td></td><td></td>
</tr>
<tr><td colspan="4"> <p> </td></tr>
<tr>
<td> </td>
<td><img align="top" src="../images/good-cd.png"></td>
<td><img align="top" src="../images/ecc.png"></td>
<td>Джейн записывает новый CD из восстановленного ISO-образа. Она хранит
данные для исправления ошибок для нового CD, так как он также может повредиться
в будущем.</td>
</table>


<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
