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
begin_page();
?>

<!--- Insert actual page content below --->

<h3>Диалоги и кнопки</h3>

В этом разделе описываются часто используемые диалоги и кнопки:

<pre> </pre>

<table width="100%">
<tr>
<td align="center"><a href="howtosa1.php"><img src="../images/good-cd.png" border="0"></a></td>
<td><a href="howtosa1.php">Меню выбора привода</a>.</td>
</tr>
<tr>
<tr><td>&nbsp;</td><td></td></tr>
<td align="center"><a href="howtosa2.php"><img src="../images/good-image.png" border="0"></a></td>
<td><a href="howtosa2.php">Окно выбора файла образа</a>.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>
<tr>
<td align="center"><a href="howtosa3.php"><img src="../images/ecc.png" border="0"></a></td>
<td><a href="howtosa3.php">Окно выбора файла для исправления ошибок</a>.</td>
</tr>
<tr><td>&nbsp;</td><td></td></tr>
<tr>
<td align="center"><a href="howtosa4.php">
  <img src="images/read-icon.png" border="0">
  <img src="images/create-icon.png" border="0"><br>
  <img src="images/scan-icon.png" border="0">
  <img src="images/fix-icon.png" border="0"><br>
  <img src="images/compare-icon.png" border="0">
  <img src="images/stop-icon.png" border="0">
</a></td>
<td><a href="howtosa4.php">Кнопки для запуска действий</a>.</td>
</tr>
</table>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
