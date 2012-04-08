<?php
# dvdisaster: Russian homepage translation
# Copyright (C) 2007-2012 Igor Gorbounov
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

if($news_flash == 0) 
{  require("../include/dvdisaster.php");
   begin_page();
}

$news_counter = 0;

news_headline("News archive from 2009");

news_item("01.10.2009", "Assessment of potential vulnerabilities in the windows version
of the GTK library", "
The Windows versions of dvdisaster 0.70.x/0.72.x are shipped with an outdated
GTK library containing vulnerabilities in its image processing routines.
To exploit the vulnerability, manipulated images need to be loaded from
an external source. Since dvdisaster does not contain/use such functions,
these vulnerabilities are not considered to be a threat.<p>

It is not recommended to replace GTK in the 0.70.x/0.72.x versions of
dvdisaster as some interfaces have been changed in newer GTK versions.
Replacing GTK will likely cause severe malfunction.<p>

The windows version of dvdisaster 0.73.1 will have updated interfaces
and will be shipped with a current version of GTK.<p>

Many thanks to all users who brought this issue to my attention.
", 13, "2009-10-01T00:00:00Z", "2009-10-01T00:00:00Z");

news_item("10.08.2009", "Project represention on SourceForge may be outdated", "
Due to the recent functionality changes on SourceForge we are unable
to upload the News, Downloads and CVS contents in time. Please refer
to the respective sections on this site (http://dvdisaster.net) instead.
", 12, "2009-08-10T00:00:00Z", "2009-08-10T00:00:00Z");

news_item("08.08.2009", "dvdisaster 0.72.1 released", "
Pablo Almeida provided a Portuguese translation for the screen text.
Added workaround to avoid Win XP freezing on certain CD-RW/drive pairs.<p>
<i>Update: The workaround is not complete. If the problem persists please
try version 0.79.x.</i>
", 11, "2009-08-08T00:00:00Z", "2010-02-06T00:00:00Z");

news_item("04.07.2009", "выпущен dvdisaster 0.72", "
Это первая стабильная версия ветки 0.72.
Игорь Горбунов завершил перевод онлайн-документации на русский язык.
Устранены некоторые незначительные ошибки в первом кандидате на выпуск.<p>
", 10, "2009-07-04T00:00:00Z", "2009-07-04T00:00:00Z");

news_item("11.04.2009", "выпущен dvdisaster 0.72.rc1", "
  Теперь доступен первый кандидат на выпуск из стабильной ветви 0.72.
  Среди новых возможностей - поддержка носителей Blu-Ray, низкоуровневое чтение и проверки на C2
  для CD, а также \"родная\" версия для Mac OS X.
  Дополнительную информацию см. здесь <a href=\"download.php\">download page</a>.
", 8, "2009-04-11T00:00:00Z", "2009-04-11T00:00:00Z");

news_item("08.03.2009", "выпущен dvdisaster 0.71.28", "
  В этой версии исправляются падения из-за \"illegal instruction\" на x86-машинах,
  которые не поддерживают SSE2. Много незначительных исправлений перед кандидатом
  на выпуск.
", 7, "2009-03-08T00:00:00Z", "2009-03-08T00:00:00Z");

news_item("18.01.2009", "выпущен dvdisaster 0.71.27", "
  Эта документация, наконец, завершена. Исправлены некоторые падения и несовместимости
  с необычными носителями и приводами. В этой версии произведен
  последний большой внутренний пересмотр, планировавшийся для версий 0.71.x, поэтому, если
  все будет хорошо, то следующей версией будет стабильный кандидат на выпуск.
", 6, "2009-01-18T00:00:00Z", "2009-01-18T00:00:00Z");

if($news_flash == 0) 
   end_page();
?>
