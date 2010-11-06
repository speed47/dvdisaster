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

<h3>Системные требования</h3>

<ul>
 <li>Процессоры: x86, PowerPC или Sparc;<p></li>
 <li>со скоростью обработки не менее, чем скорость P4 при 2ГГц;<p></li>
 <li>современный привод CD/DVD/HD DVD/BD с интерфейсом ATAPI или SCSI;<p></li>
 <li>достаточное пространство на жестком диске для создания .iso-образов обрабатываемых носителей.<p>
</ul>

<h3>Операционные системы</h3>
<ul>
 <li><a name="#freebsd"></a><b>FreeBSD</b> версия <b>6.0</b> или новее<br>
     (для использования ATAPI-приводов требуется перекомпиляция ядра -- см. файл INSTALL)<p>
 </li>
 <li><b>Linux</b> с ядром <b>2.6.7</b> или новее.<p>
 </li>
 <li><b>Mac OS X</b> версия 10.4 (Tiger) или новее,<br> 
      на платформах x86 и PowerPC.<p>
 <li><b>NetBSD</b> версия 3.1 или новее.<p></li> 
 <li><b>Windows 2000</b>, <b>Windows XP</b>, <b>Windows Vista (R).</b></li>
 </li>
</ul>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
