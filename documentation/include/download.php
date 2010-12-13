<?php

# dvdisaster: Download layout functions
# Copyright (C) 2010 Carsten Gnörlich

function download_version($version, $show_separator, $src_md5, $mac_md5, $win_md5)
{  global $trans_sourcecode;
   global $trans_signature;
   global $trans_md5;
   global $trans_macbinary;
   global $trans_winbinary;
   global $trans_version;

  if($show_separator)
    echo "    <tr><td></td><td>$trans_version $version</td></tr>\n";

  echo "    <tr><td align=\"right\">&nbsp;&nbsp;$trans_sourcecode&nbsp;</td>\n";
  echo "        <td><a href=\"http://dvdisaster.net/downloads/dvdisaster-$version.tar.bz2\">dvdisaster-$version.tar.bz2</a></td></tr>\n";
  echo "    <tr><td align=\"right\">$trans_signature&nbsp;</td>\n";
  echo "        <td><a href=\"http://dvdisaster.net/downloads/dvdisaster-$version.tar.bz2.gpg\">dvdisaster-$version.tar.bz2.gpg</a></td></tr>\n";

  if($src_md5 != "hidden")
    echo "<tr><td align=\"right\">$trans_md5&nbsp;</td><td>$src_md5</td></tr>\n";
  echo "    <tr><td colspan=\"2\"><img width=1 height=3</td></tr>\n";

  if($mac_md5 != "none")
  {  echo "    <tr><td align=\"right\">$trans_macbinary&nbsp;</td>\n";
     echo "        <td><a href=\"http://dvdisaster.net/downloads/dvdisaster-$version.app.zip\">dvdisaster-$version.app.zip</a></td></tr>\n";
     echo "    <tr><td align=\"right\">$trans_signature&nbsp;</td>\n";
     echo "        <td><a href=\"http://dvdisaster.net/downloads/dvdisaster-$version.app.zip.gpg\">dvdisaster-$version.app.zip.gpg</a></td></tr>\n";

     if($mac_md5 != "hidden")
       echo "<tr><td align=\"right\">$trans_md5&nbsp;</td><td>$mac_md5</td></tr>\n";
     echo "    <tr><td colspan=\"2\"><img width=1 height=3</td></tr>\n";
  }

  if($win_md5 != "none")
  {  echo "    <tr><td align=\"right\">$trans_winbinary&nbsp;</td>\n";
     echo "        <td><a href=\"http://dvdisaster.net/downloads/dvdisaster-$version-setup.exe\">dvdisaster-$version-setup.exe</a></td></tr>\n";
     echo "    <tr><td align=\"right\">$trans_signature&nbsp;</td>\n";
     echo "        <td><a href=\"http://dvdisaster.net/downloads/dvdisaster-$version-setup.exe.gpg\">dvdisaster-$version-setup.exe.gpg</a></td></tr>\n";

     if($win_md5 != "hidden")
       echo "<tr><td align=\"right\">$trans_md5&nbsp;</td><td>$win_md5</td></tr>\n";
     echo "    <tr><td colspan=\"2\"> </td></tr>\n";
  }
}