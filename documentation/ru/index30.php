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
?>

<!-- Insert actual page content below -->

<h3 class="top">Преимущества использования dvdisaster:</h3>

<ul>
<li><b>Защищает</b> от старения и случайного повреждения носителя (в определенных пределах).<p></li>
<li><a href="howtos10.php">Проверки на ошибки чтения</a> проводятся <b>быстрее</b>, чем проверки качества носителя;
вплоть до полной скорости чтения, в зависимости от привода.<p></li>
<li><b>Экономия затрат:</b> Носители должны заменяться новой копией, 
только когда они действительно повреждены.</li>
</ul>

<h3>Ограничения при использовании программы dvdisaster:</h3>

Требуется стратегия резервирования и по меньшей мере 15% дополнительного места.

<ul>
<li>Данные для исправления ошибок <b>должны быть созданы до того, как носитель выйдет из строя</b>, 
желательно тогда же, когда записывается носитель.<p></li>
<li>Данным для исправления ошибок требуется <b>дополнительное место</b>, либо на защищаемом 
носителе, либо на дополнительных носителях. 
При стандартных настройках дополнительное место
составляет до 15% первоначального размера данных
(приблиз. 700 МБ для полного 4.7 Гб DVD).<p></li>
<li>нет гарантированной защиты от потери данных.</li>
</ul>

Смотрите также подборку <a href="http://dvdisaster.net/legacy/en/background.html">справочной информации</a>,
чтобы узнать больше о том, как работает dvdisaster.

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
