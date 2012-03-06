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

Please visit the <a href="http://dvdisaster.net/en/download40.php">online version of these pages</a> for currently available alpha versions. 

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
