<?php
# dvdisaster: Russian homepage translation
# Copyright (C) 2007-2009 Igor Gorbounov
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

<h3>Цифровая подпись</h3>

Выложенные для скачивания пакеты dvdisaster снабжены цифровой подписью с помощью
<a href="http://www.gnupg.org/gnupg.html">GnuPG</a>, поэтому вы можете проверить
целостность пакетов.<p>


Подпись была сделана с помощью следующего <a href="../pubkey.asc">открытого ключа</a>:

<a href="../pubkey.asc">
<pre>
pub   1024D/F5F6C46C 2003-08-22
      Key fingerprint = 12B3 1535 AF90 3ADE 9E73  BA7E 5A59 0EFE F5F6 C46C
uid                  dvdisaster (pkg signing key #1)
sub   1024g/091AD320 2003-08-22
</pre></a>

Пишите на <img src="../images/email.png" align="top"> и вы получите
отпечаток ключа непосредственно от разработчиков. 
В тему письма вставьте строку "GPG finger print".

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
