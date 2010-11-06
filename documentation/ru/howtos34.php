<?php
# dvdisaster: English homepage translation
# Copyright (C) 2004-2010 Carsten Gnörlich
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

require("../include/dvdisaster.php");
require("../include/screenshot.php");
begin_page();

howto_headline("Дополнение образов данными для исправления ошибок", "Дополнительные параметры", "images/create-icon.png");
?>

<?php begin_screen_shot("Вкладка \"Исправление ошибок\".","create-prefs-ecc2-adv.png"); ?>
<b>Выбор размера образа</b>. В dvdisaster'е есть таблица стандартных размеров носителей CD, DVD и BD. Любой носитель должен удовлетворять этим требованиям к размерам.
Некоторые производители изготавливают носители слегка большей емкости. Если у вас такие носители, вставьте пустой носитель в выбранный привод и нажмите кнопку
"запросить носитель" (помечено зеленым) напротив подходящего типа носителя.
dvdisaster определит размер носителя и обновит соответствующим образом эту таблицу.<p>
<b>Примечание:</b> Размер носителя может быть определен только в приводах, которые могут писать на соответствующих типах носителей.
<pre> </pre>
<b>Произвольные размеры носителей.</b> Можно задать специальный размер образа, который не будет превышен после добавления в него данных для исправления ошибок.
Чтобы это сделать, активируйте кнопку "Использовать не больше ... секторов" и введите максимальный размер образа в секторах (1 сектор = 2КБ).
<?php end_screen_shot(); ?>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
