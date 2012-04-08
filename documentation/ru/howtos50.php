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
howto_headline("Получение информации об образах и данных для исправления ошибок", "Обзор", "images/compare-icon.png");
?>

<!-- Insert actual page content below -->

<table width="100%" cellspacing="5">
<tr valign="top">
<td class="w20p"><b>Задача</b></td>
<td>
Выводит информацию о типах и состоянии образов и файлов для исправления ошибок.
</td>
</tr>
<tr><td> <pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Требуется:</b><p></td>
</tr>
<tr>
 <td class="w150x" align="right">
   <img src="../images/good-image.png" alt="Icon: Complete image" class="valignt">
   <img src="../images/ecc.png" alt="Icon: Separate file with error correction data">
 </td>
<td>
Файл образа и, возможно, файл исправления ошибок для него.
</td>
<tr><td> <pre> </pre> </td></tr>

<tr>
<td colspan="2"><b>Что делать:</b><p></td>
</tr>

<tr>
<td></td>
<td>
1. <a href="howtos51.php">Показать информацию</a><p>
2. <a href="howtos51.php#examine">Объяснить результаты</a>
</td>
</tr>
</table><p>

<pre>


</pre>

<a href="howtos51.php">Показать информацию...</a>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
