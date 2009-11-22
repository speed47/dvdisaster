<?php

# dvdisaster: Homepage layout funtions
# Copyright (C) 2007-2009 Carsten Gnörlich

require("version.php");

# Preset some global variables

$project_at_hoster="http://sourceforge.net/projects/dvdisaster";

# Find out from where we have been called;
# the file name is important for creation of the index.
# Also we extract the current locale from the path which is supposed
# to be .../LC_php/file.php. Maybe this is too hackish...

$script_path = current(get_included_files());
$script_file = basename($script_path);
$script_name = basename($script_path, ".php");
$script_dir  = dirname($script_path);
$script_lang = substr($script_dir, strlen($script_dir)-2, 2);

# Load the appropriate localization file

require("dict_" . $script_lang . ".php");

#
# Create the HTML header ----------------------------------------------------------
#

function start_html()
{  echo "<html>\n";
   echo "<head>\n";
   echo " <meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n";
   echo " <title>dvdisaster</title>\n";
   echo " <link rel=\"stylesheet\" type=\"text/css\" href=\"../include/dvdisaster.css\">\n";

   echo "</head>\n";
   echo "<body>\n";
}

###
### Head bar ------------------------------------------------------------------------
###
# This includes the "dvdisaster version ..." header. Depending on the mode we
# - provide the link to the online version for local documentation, or
# - create the language switch for the online publication.

function lang_link($lang_name, $lang, $spacing)
{  global $script_file;
   global $script_lang;
   
   if($spacing) $space="&nbsp;&nbsp;&nbsp;";
   else         $space="";

   if(strcmp($lang, $script_lang)) 
        echo "       <a href=\"../${lang}/$script_file\">$lang_name</a> $space\n";
   else echo "       $lang_name $space\n";
}

function begin_page()
{  global $cooked_version;
   global $trans_to_hoster;
   global $trans_to_internet;
   global $trans_version;
   global $project_at_hoster;
   global $script_lang;
   global $mode;

   # Begin HTML file

   start_html();

   # Write the title header

   echo "\n<!--- Title header --->\n";
   echo "<table width=\"100%\" cellpadding=\"0\" border=\"0\">\n";
   echo "  <tr>\n";
   echo "     <td align=\"left\">\n";
   echo "       <font size=\"+3\"><b>dvdisaster</b></font>\n";
   echo "       <i>$trans_version $cooked_version</i>\n";

   if(!strcmp($mode, "local"))
   {  echo "  </td>\n";
      echo "  <td align=\"right\">\n";
      echo "     <font size=\"+3\">&nbsp;</font><a href=\"http://dvdisaster.net/$script_lang/\">$trans_to_internet</a>\n";
      lang_link("", "de", 0); # TODO: This is a quick hack
      lang_link("", "en", 0); # to produce all locales for
      lang_link("", "ru", 0); # Windows. Do it better!
   }
   echo "     </td>\n";
   echo "  </tr>\n";
   echo "</table>\n";

   # Write the language chooser depending on mode

   echo "\n<!--- Language chooser or separator --->\n";
   echo "<table width=\"100%\" cellpadding=\"0\" border=\"0\">\n";

   echo "  <tr bgcolor=\"#000000\">\n";
   echo "    <td colspan=\"2\" width=\"100%\"><img width=1 height=1 alt=\"\"></td>\n";
   echo "  </tr>\n";

   if(strcmp($mode, "local"))
   {  
      echo "  <tr>\n";
      echo "    <td align=\"left\"><a href=\"$project_at_hoster\">$trans_to_hoster</a></td>\n";
      echo "    <td align=\"right\">\n";
#      lang_link("&#268;esky", "cs");
      lang_link("Deutsch", "de", 1);
      lang_link("English", "en", 1);
      lang_link("Russian", "ru", 0);
      echo "    </td>\n";
      echo "  </tr>\n";
      echo "  <tr bgcolor=\"#000000\">\n";
      echo "    <td colspan=\"2\" width=\"100%\"><img width=1 height=1 alt=\"\"></td>\n";
      echo "  </tr>\n";
   }

   # Both modes get the separator 

   echo "  <tr><td colspan=\"2\" width=\"100%\" height=\"10\">\n";
   echo "     <img width=1 height=1 alt=\"\">\n";
   echo "  </td></tr>\n";
   echo "</table>\n";

   # Insert the navigation column

   create_navigation();
}

###
### Navigation column ----------------------------------------------------------------
###

#
# Helper functions for creating the table of contents
#

function section($section_name)
{  global $toc_section;
   global $toc_mode;

   $toc_section = $section_name;
   $toc_mode = "section";
}

function subsection($subsection_name)
{  global $toc_subsection;
   global $toc_mode;

   $toc_subsection = $subsection_name;
   $toc_mode = "subsection";
}

function subsubsection($subsubsection_name)
{  global $toc_subsubsection;
   global $toc_mode;

   $toc_subsubsection = $subsubsection_name;
   $toc_mode = "subsubsection";
}

function toc_link($msg, $lang)
{  static $separator=0;
   global $script_lang;
   global $script_name;
   global $toc_section;
   global $toc_subsection;
   global $toc_subsubsection;
   global $toc_mode;

   if(strcmp($lang, $script_lang)) return; # wrong locale

   # Decide whether this is the currently unfolded (sub)section
   # and render it accordingly

   if(!strcmp($toc_mode, "section"))
   {  $target=$toc_section . ".php";

      # Draw the separator between sections (except for the first one)

      if(!$separator) $separator=1;
      else echo "            <tr><td></td><td></td><td height=\"10\"></td></tr>\n";

      if(!strcmp($toc_section, $script_name))
	   echo "            <tr><td colspan=3><font size=\"-1\">$msg</font></td></tr>\n";
      else echo "            <tr><td colspan=3><font size=\"-1\"><a href=\"$target\">$msg</a></font></td></tr>\n";
   }

   if(   !strcmp($toc_mode, "subsection") 
      && !strncmp($toc_section, $script_name, strlen($toc_section)))
   {  $target=$toc_subsection . ".php";

      if(!strcmp($toc_subsection, $script_name))
	   echo "            <tr><td valign=\"top\" width=\"1%\">&middot;</td><td colspan=2><font size=\"-1\">$msg</font></td></tr>\n";
      else echo "            <tr><td valign=\"top\" width=\"1%\">&middot;</td><td colspan=2><font size=\"-1\"><a href=\"$target\">$msg</a></font></td></tr>\n";
   }

   # Using strlen($toc_section)+1 is inconvenient as it hardcodes the
   # section/subsection10/subsubsection11 scheme. Improve!

   if(   !strcmp($toc_mode, "subsubsection") 
      && !strncmp($toc_subsection, $script_name, strlen($toc_section)+1))
   {  $target=$toc_subsubsection . ".php";

      if(!strcmp($toc_subsubsection, $script_name))
	   echo "            <tr><td valign=\"top\"></td><td>-</td><td><font size=\"-1\">${msg}</font></td></tr>\n";
      else echo "        <tr><td valign=\"top\"></td><td>-</td><td><font size=\"-1\"><a href=\"$target\">$msg</a></font></td></tr>\n";
   }

}

function de($msg) {toc_link($msg, "de"); };
function en($msg) {toc_link($msg, "en"); };
function ru($msg) {toc_link($msg, "ru"); };

#
# Helper functions for creating the news pages and -flash
#

function news_headline($headline)
{  global $news_flash;

   if(!$news_flash) echo "    <h3>$headline</h3>\n";
}

function news_item($date, $headline, $body)
{  global $news_flash;
   global $news_counter;

   $news_counter++;

   if($news_flash)
   {  echo "          <font size=\"-1\">$date</font> <br>\n";
      echo "          <font size=\"-1\">\n";
      echo "            <a href=\"news.php#item$news_counter\">$headline</a>\n";
      echo "          </font><p>\n";
   }
   else
   {  
      echo "    <table width=\"90%\">\n";
      echo "      <tr>\n";
      echo "        <td><a name=\"item$news_counter\"></a><b>${headline}</b></td>\n";
      echo "        <td align=\"right\">$date</td>\n";
      echo "      </tr>\n";
      echo "    </table>\n";
?>
    <table width="90%" cellpadding="0" cellspacing="0">
       <tr bgcolor="#000000" height=1>
	 <td width="100%" height=1><img width=1 height=1 alt=""></td>
       </tr>
    </table>
    <table width="90%" cellpadding="0" cellspacing="0">
       <tr bgcolor="#ffffff" height=5>
         <td width="100%" height=1><img width=1 height=1 alt=""></td>
       </tr>
    </table>
<?php
      echo "    <table width=\"90%\">\n";
      echo "      <tr><td>\n";
      echo "$body\n";
      echo "      </td></tr>\n";
      echo "    </table>\n";
      echo "    <pre> </pre>\n";
   }
}

#
# Actual navigation assembly
#

function create_navigation()
{  global $trans_contents;
   global $script_file;
   global $mode;   

   if(!strcmp($script_file, "index.php"))
        $body_width = "57%";
   else $body_width = "77%";
?>
  
<!--- Main body (Navigation, actual page content, optional news column) ---> 
<table width="100%" cellspacing=0>
 <tr>
  <!--- Navigation --->
  <td bgcolor="#f0f0f0" valign="top" width="20%">
    <table width="100%" cellpadding="10">
      <tr>
        <td>
<?php
echo "         <font size=\"-1\"><b>$trans_contents</b></font>\n";
?>
          <table width="100%" cellpadding="0" cellspacing="0">
	    <tr bgcolor="#000000">
	      <td width="100%"><img width=1 height=1 alt=""></td>
	    </tr>
	  </table><p>

          <table width="100%">
<?php require("toc.php");?>
	  </table>
        </td>
      </tr>
    </table>
  </td>
  <td></td>

  <!--- Actual page contents --->
<?php
echo "  <td valign=\"top\" width=\"$body_width\" rowspan=\"2\">\n";

  # body contents must be appended here from calling page
}

###
### Footer ----------------------------------------------------------------------------
### 
# Closes the body table. Optionally includes the news flash.
# Appends the copyright and redistribution terms.
#

function end_page()
{  global $script_file;
   global $trans_copyright;
   global $trans_modified;
   global $trans_news;
   global $trans_fdl;
   global $trans_hosting;
   global $modified_source;
   global $news_flash;
   global $script_lang;  /* for old version link */
   global $trans_old_version;

# Close the body table

  echo "  </td> <!--- end of page contents --->\n";

# Insert news flash

  if(!strcmp($script_file, "index.php"))
  {
?>

  <!--- news flash column --->
  <td></td>
  <td bgcolor="#e0e0ff" valign="top" width="20%">
    <table width="100%" cellpadding="10"><tr><td>
<?php
  echo "      <font size=\"-1\"><b>$trans_news</b></font>\n";
?>
      <table width="100%" cellpadding="0" cellspacing="0">
         <tr bgcolor="#000000">
	   <td width="100%"><img width=1 height=1 alt=""></td>
	 </tr>
      </table><p>
      <table width="100%">
        <tr><td>
<?php
	  $news_flash=1;
	  require("news.php");
?>
        </td></tr>
      </table>
    </table>
  </td> <!--- end of news flash column--->
<?php
  }
# Reference to our hoster
/*
 </tr>
 <tr valign="bottom">
   <td bgcolor="#f0f0f0" align="center">
<?php echo "     <font size=\"-2\">$trans_hosting</font><br>\n"; ?>
      <img src="../images/mokelbude.png"
           width="125" height="37" border="0" alt="insider">
   </td>
   <td></td>
   <td></td>
*/
?>

 </tr>
 <tr valign="bottom">
   <td bgcolor="#f0f0f0">
<?php 
   $old_lang = strcmp($script_lang, "ru") ? $script_lang : "en";
   echo "      <table cellpadding=\"10\"><tr><td><a href=\"http://dvdisaster.net/legacy/$old_lang/index.html\"><font size=\"-1\">$trans_old_version</font></a></td></tr></table>\n"; 
?>
   </td>
   <td></td>
   <td></td>

 </tr>
</table> <!--- end of main body table --->

<?php
# Create the footer
?>

<!--- Page footer --->
<table width="100%" cellpadding="0" border="0">
 <tr><td colspan="2" width="100%" height="10"><img width=1 height=1 alt=""></td>
 </tr>
 <tr bgcolor="#000000"><td colspan="2" width="100%"><img width=1 height=1 alt=""></td>
 </tr>
 <tr>
  <td align="center">
   <font size="-1">
<?php
   echo "     <i> $trans_copyright<br>\n";
   echo "         $trans_fdl\n";
?>
    </i>
   </font>
  </td>
 </tr>
 <tr bgcolor="#000000"><td colspan="2" width="100%"><img width=1 height=1 alt=""></td>
 </tr>
</table>
</body>
</html>
<?php
}

#
# Special headings etc
#

function howto_headline($headline, $subtitle, $image)
{
   echo "<table width=\"100%\" bgcolor=\"#f0f0f0\">\n";
   echo "<tr><td><font size=\"+1\"><b>$headline</b></font></td>\n";
   echo "<td rowspan=\"2\" align=\"right\"><img src=\"$image\"></td></tr>\n";
   echo "<tr><td><i>$subtitle</i></td></tr>\n";
   echo "</table><p>\n";
}
?>
