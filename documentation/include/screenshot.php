<?php
#  dvdisaster homepage: Screen shot helper routines
#  Copyright (C) 2007-2010 Carsten Gn�rlich
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA,
#  or direct your browser at http://www.gnu.org.

$screen_shot_counter = 0;

#
# Screen shots for showing preferences windows
#

function begin_screen_shot($caption, $image)
{  global $screen_shot_counter;
   global $screen_shot_image;
   global $screen_shot_caption;
   global $screen_shot_expand;
   global $screen_shot_way;

   $screen_shot_expand=$_GET["expand"];
   $screen_shot_way=$_GET["way"];
   $screen_shot_counter++;
   $screen_shot_image = $image;
   $screen_shot_caption = $caption;

   echo "<!--- begin of screen shot --->\n";

   # Screen version (large image)

   if($screen_shot_counter == $screen_shot_expand)
   {  echo "<table class=\"screenshot\">\n";
      echo "  <tr><td>\n";
      echo "    <a name=\"snap$screen_shot_counter\"></a>\n";
   }

   # Thumbnail version

   else
   {  echo "<table>\n";
      echo "  <tr>\n";
      echo "    <td valign=\"top\">\n";
   }
}

function end_screen_shot()
{  global $screen_shot_counter;
   global $screen_shot_image;
   global $screen_shot_caption;
   global $screen_shot_expand;
   global $screen_shot_way;
   global $script_file;
   global $trans_screen_shot;

   if($screen_shot_counter == $screen_shot_expand)
   {  echo "  </td></tr>\n";
      echo "  <tr><td align=\"center\">\n";
      if($screen_shot_way != "")
	   echo "    <a href=\"${script_file}?way=$screen_shot_way&#snap${screen_shot_counter}\">\n";
      else echo "    <a href=\"${script_file}#snap${screen_shot_counter}\">\n";
      echo "      <img src=\"images/$screen_shot_image\">\n";
      echo "    </a>\n";
      echo "  </td></tr>\n";
      echo "  <tr><td align=\"center\">$trans_screen_shot: $screen_shot_caption</td></tr>\n";
      echo "</table>";
      echo "<pre> </pre>\n";
   }
   else
   {  echo "    </td>\n";
      echo "    <td>\n";
      echo "      <a name=\"snap${screen_shot_counter}\"></a>\n";
      echo "      <table class=\"thumbnail\" width=\"160px\">\n";
      echo "        <tr><td><font size=\"-1\">$screen_shot_caption</font></td></tr>\n";
      if($screen_shot_way != "")
	   echo "        <tr><td><a href=\"${script_file}?way=$screen_shot_way&expand=${screen_shot_counter}#snap${screen_shot_counter}\">";
      else echo "        <tr><td><a href=\"${script_file}?expand=${screen_shot_counter}#snap${screen_shot_counter}\">";
      echo "<img src=\"thumbnails/$screen_shot_image\"></a>";
      echo "</td></tr>\n";
      echo "      </table>\n";
      echo "    </td>\n";
      echo "  </tr>\n";
      echo "</table><p>\n";
   }

   echo "<!--- end of screen shot --->\n";
}


#
# Smaller screen shots for the howtos
#

function begin_howto_shot($caption, $image, $arrow)
{  global $screen_shot_counter;
   global $screen_shot_image;
   global $screen_shot_caption;
   global $screen_shot_expand;
   global $screen_shot_arrow;
   global $screen_shot_way;

   $screen_shot_expand=$_GET["expand"];
   $screen_shot_way=$_GET["way"];
   $screen_shot_counter++;
   $screen_shot_image = $image;
   $screen_shot_caption = $caption;
   $screen_shot_arrow = $arrow;

   echo "<!--- begin of screen shot --->\n";

   # Screen version (large image)

   if($screen_shot_counter == $screen_shot_expand)
   {  echo "<table class=\"screenshot\">\n";
      echo "  <tr><td>\n";
      echo "    <a name=\"snap$screen_shot_counter\"></a>\n";
   }

   # Thumbnail version

   else
   {  echo "<table>\n";
      echo "  <tr>\n";
      echo "    <td width=\"200px\" align=\"center\">\n";
      echo "      <a name=\"snap${screen_shot_counter}\"></a>\n";
      if($screen_shot_way != "")
	   echo "      <a href=\"${script_file}?way=$screen_shot_way&expand=${screen_shot_counter}#snap${screen_shot_counter}\">";
      else echo "      <a href=\"${script_file}?expand=${screen_shot_counter}#snap${screen_shot_counter}\">";
      echo "       <img src=\"thumbnails/$image\" border=\"0\">\n";
      if($screen_shot_arrow != "")
	 echo "       <br><img src=\"../images/$arrow\" border=\"0\">\n";
      echo "      </a>\n";
      echo "    </td><td>&nbsp; &nbsp;</td>\n";
      echo "    <td valign=\"top\">\n";
   }
}

function end_howto_shot()
{  global $screen_shot_counter;
   global $screen_shot_image;
   global $screen_shot_caption;
   global $screen_shot_expand;
   global $screen_shot_way;
   global $screen_shot_arrow;
   global $script_file;
   global $trans_screen_shot;

   if($screen_shot_counter == $screen_shot_expand)
   {  echo "  </td></tr>\n";
      echo "  <tr><td align=\"center\">\n";
      if($screen_shot_way != "")
	   echo "    <a href=\"${script_file}?way=$screen_shot_way&expand=0#snap${screen_shot_counter}\">\n";
      else echo "    <a href=\"${script_file}?expand=0#snap${screen_shot_counter}\">\n";
      echo "      <img src=\"images/$screen_shot_image\">\n";
      echo "    </a>\n";
      echo "  </td></tr>\n";
      echo "  <tr><td align=\"center\">$trans_screen_shot: $screen_shot_caption</td></tr>\n";
      echo "</table>";

      if($screen_shot_arrow != "")
      {  	 echo "<table>\n";
	 echo "  <tr>\n";
	 echo "    <td width=\"200px\" align=\"center\">\n";
	 echo "      <img src=\"../images/$screen_shot_arrow\" border=\"0\">\n";
	 echo "    </td>\n";
	 echo "  </tr>\n";
	 echo "</table>\n";
      }
   }
   else
   {  echo "    </td>\n";
      echo "  </tr>\n";
      echo "</table><p>\n";
   }

   echo "<!--- end of screen shot --->\n";
}
?>