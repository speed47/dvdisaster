<?php

# dvdisaster: Homepage layout functions
# Copyright (C) 2007-2012 Carsten Gnörlich

require("version.php");

# Preset some global variables

$project_at_hoster="http://sourceforge.net/projects/dvdisaster";
$max_news_flash_items = 7;
$create_feed = 0;

# Find out from where we have been called;
# the file name is important for creation of the index.
# Also we extract the current locale from the path which is supposed
# to be .../LC_php/file.php. Maybe this is too hackish...

$script_path = current(get_included_files());
$script_file = basename($script_path);
$script_name = basename($script_path, ".php");
$script_dir  = dirname($script_path);
$script_lang = substr($script_dir, strlen($script_dir)-2, 2);

# Needed to exatract some meaningful title text from the toc.php

$toc_title_mode = 0;
$toc_title_content = "unset title";

# Load the appropriate localization file

require("dict_" . $script_lang . ".php");

# Locale wrappers for toc.php stubs

function cs($msg) 
{  global $toc_title_mode; 

   if($toc_title_mode == 1) toc_title($msg, "cs"); 
   else                     toc_link($msg, "cs"); 
};

function de($msg) 
{  global $toc_title_mode; 

   if($toc_title_mode == 1) toc_title($msg, "de"); 
   else                     toc_link($msg, "de"); 
};

function en($msg) 
{  global $toc_title_mode; 

   if($toc_title_mode == 1) toc_title($msg, "en"); 
   else                     toc_link($msg, "en");
};

function ru($msg) 
{  global $toc_title_mode; 

   if($toc_title_mode == 1) toc_title($msg, "ru"); 
   else                     toc_link($msg, "ru");
};

#
# Create the HTML header ----------------------------------------------------------
#

function start_html()
{  global $toc_title_mode;
   global $toc_title_content;
   global $script_name;
   global $script_lang;
   global $create_feed;
   global $trans_atom_title;

   echo "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n";
   echo "<html>\n";
   echo "<head>\n";
   echo " <meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n";
   $toc_title_mode = 1;
   $toc_title_content = "dvdisaster";
   require("toc.php");
   echo " <title>$toc_title_content</title>\n";
   $toc_title_mode = 0;
   echo " <link rel=\"stylesheet\" type=\"text/css\" href=\"../include/dvdisaster.css\">\n";
   if(!strcmp($script_name, "index"))
   {  echo "<link rel=\"alternate\" type=\"application/atom+xml\" href=\"http://dvdisaster.net/$script_lang/feed/atom.xml\" title=\"$trans_atom_title\">\n";
      $create_feed=1;
   }

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
   global $have_experimental;
   global $stable_version;
   global $trans_to_hoster;
   global $trans_to_internet;
   global $trans_version;
   global $project_at_hoster;
   global $script_lang;
   global $mode;

   # Begin HTML file

   start_html();

   # Write the title header

   echo "\n<!-- Title header -->\n";
   echo "<table width=\"100%\" cellpadding=\"0\" border=\"0\">\n";
   echo "  <tr>\n";
   echo "     <td align=\"left\">\n";
   echo "       <span class=\"fxxl\"><b>dvdisaster</b></span>\n";
   if(!strcmp($have_experimental, "no"))
      echo "       <i>$trans_version $cooked_version</i>\n";
   else
      echo "       <i>$trans_version $stable_version / $cooked_version</i>\n";
   if(!strcmp($mode, "local"))
   {  echo "  </td>\n";
      echo "  <td align=\"right\">\n";
      echo "     <span class=\"fxxl\">&nbsp;</span><a href=\"http://dvdisaster.net/$script_lang/\">$trans_to_internet</a>\n";
      lang_link("", "cs", 0); # TODO: This is a quick hack
      lang_link("", "de", 0); # 
      lang_link("", "en", 0); # to produce all locales for
      lang_link("", "ru", 0); # Windows. Do it better!
   }
   echo "     </td>\n";
   echo "  </tr>\n";
   echo "</table>\n";

   # Write the language chooser depending on mode

   echo "\n<!-- Language chooser or separator -->\n";
   echo "<table width=\"100%\" cellpadding=\"0\" border=\"0\">\n";

   echo "  <tr>\n";
   echo "    <td colspan=\"2\" class=\"hsep\"></td>\n";
   echo "  </tr>\n";

   if(strcmp($mode, "local"))
   {  
      echo "  <tr>\n";
      echo "    <td align=\"left\"><a href=\"$project_at_hoster\">$trans_to_hoster</a></td>\n";
      echo "    <td align=\"right\">\n";
      lang_link("&#268;esky", "cs", 1);
      lang_link("Deutsch", "de", 1);
      lang_link("English", "en", 1);
      lang_link("Русский", "ru", 0);
      echo "    </td>\n";
      echo "  </tr>\n";
      echo "  <tr>\n";
      echo "    <td colspan=\"2\" class=\"hsep\"></td>\n";
      echo "  </tr>\n";
   }

   # Both modes get the separator 

   echo "  <tr><td colspan=\"2\" style=\"height: 10px\">\n";
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

function toc_title($msg, $lang)
{  global $script_lang;
   global $script_name;
   global $toc_mode;
   global $toc_section;
   global $toc_subsection;
   global $toc_subsubsection;
   global $toc_title_content;

   if(strcmp($lang, $script_lang)) return; # wrong locale

   if(!strcmp($toc_mode, "section") && !strcmp($toc_section, $script_name))
     $toc_title_content = $msg;

   if(!strcmp($toc_mode, "subsection") && !strcmp($toc_subsection, $script_name))
     $toc_title_content = $msg;

   if(!strcmp($toc_mode, "subsubsection") && !strcmp($toc_subsubsection, $script_name))
     $toc_title_content = $msg;
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

   # Decide whether this is the currently unfolded section
   # and render it accordingly

   if(!strcmp($toc_mode, "section"))
   {  $target=$toc_section . ".php";

      # Draw the separator between sections (except for the first one)

      if(!$separator) $separator=1;
      else echo "            <tr><td></td><td></td><td style=\"height:10px;\"></td></tr>\n";

      if(!strcmp($toc_section, $script_name))
	   echo "            <tr><td colspan=3><span class=\"fs\">$msg</span></td></tr>\n";
      else echo "            <tr><td colspan=3><span class=\"fs\"><a href=\"$target\">$msg</a></span></td></tr>\n";
   }

   if(   !strcmp($toc_mode, "subsection") 
      && !strncmp($toc_section, $script_name, strlen($toc_section)))
   {  $target=$toc_subsection . ".php";

      if(!strcmp($toc_subsection, $script_name))
	   echo "            <tr><td style=\"width:1%; vertical-align:top;\">&middot;</td><td colspan=2><span class=\"fs\">$msg</span></td></tr>\n";
      else echo "            <tr><td style=\"width:1%; vertical-align:top;\">&middot;</td><td colspan=2><span class=\"fs\"><a href=\"$target\">$msg</a></span></td></tr>\n";
   }

   # Using strlen($toc_section)+1 is inconvenient as it hardcodes the
   # section/subsection10/subsubsection11 scheme. Improve!

   if(   !strcmp($toc_mode, "subsubsection") 
      && !strncmp($toc_subsection, $script_name, strlen($toc_section)+1))
   {  $target=$toc_subsubsection . ".php";

      if(!strcmp($toc_subsubsection, $script_name))
	   echo "            <tr><td></td><td style=\"vertical-align:top; font-size:small;\">-</td><td><span class=\"fs\">${msg}</span></td></tr>\n";
      else echo "        <tr><td></td><td style=\"vertical-align:top; font-size:small;\">-</td><td><span class=\"fs\"><a href=\"$target\">$msg</a></span></td></tr>\n";
   }

}

#
# Helper functions for creating the news pages and -flash
#

function news_headline($headline)
{  global $news_flash;
   global $atom_handle;
   global $create_feed;
   global $script_lang;
   global $doc_dir;
   global $trans_atom_title;

   if(!$news_flash) echo "    <h3 class=\"top\">$headline</h3>\n";

   if($create_feed != 1) return;

   # Prodoce atom feed xml file

   $atom_name="atom.xml";
   $atom_handle=fopen("$doc_dir/$script_lang/feed/$atom_name","w");
   
   fwrite($atom_handle, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n");
   fwrite($atom_handle, "<feed xmlns=\"http://www.w3.org/2005/Atom\">\n");
   fwrite($atom_handle, "<id>tag:dvdisaster.net,2009-10-02:/$script_lang/feeds/$atom_name</id>\n");
   fwrite($atom_handle, "<title>$trans_atom_title</title>\n");
   $updated=date(DATE_ATOM);
   fwrite($atom_handle, "<updated>$updated</updated>\n");
   fwrite($atom_handle, "<link rel=\"self\" href=\"http://dvdisaster.net/$script_lang/feed/$atom_name\" type=\"application/atom+xml\" />\n");
   fwrite($atom_handle, "<author>\n");
   fwrite($atom_handle, " <name>Carsten Gnörlich</name>\n");
   fwrite($atom_handle, " <uri>http://www.dvdisaster.org</uri>\n");
   fwrite($atom_handle, "</author>\n");
}

function news_finalize()
{  global $atom_handle;
   global $create_feed;

   if($create_feed != 1)
     return;

   fwrite($atom_handle, "</feed>\n");
   fclose($atom_handle);
}

function news_item($date, $headline, $body, $atom_tag, $atom_created, $atom_updated)
{  global $news_flash;
   global $news_counter;
   global $max_news_flash_items;
   global $atom_handle;
   global $create_feed;
   global $script_lang;

   $news_counter++;

   if($create_feed == 1)
   {  $stripped=strtr(strip_tags($body),"\n"," ");
      $summary=substr($stripped, 0, 240);
      $cutpos=240-strlen(strrchr($summary, " "));
      $summary=substr($stripped, 0, $cutpos)." [...]";      

      fwrite($atom_handle,"<entry>\n");
      fwrite($atom_handle,"<title>$headline</title>\n");
      fwrite($atom_handle,"<category term=\"News\"/>\n"); 
      $created=substr($atom_created,0,10);
      fwrite($atom_handle,"<id>tag:dvdisaster.net,$created:/$script_lang/news.html/$atom_tag</id>\n");
      fwrite($atom_handle,"<published>$atom_created</published>\n");
      fwrite($atom_handle,"<updated>$atom_updated</updated>\n");
      fwrite($atom_handle,"<link href=\"http://dvdisaster.net/$script_lang/news.html#item$atom_tag\"/>\n");
      fwrite($atom_handle,"<summary>$summary</summary>\n");
      fwrite($atom_handle,"</entry>\n");
   }

   if($news_flash)
   {  if($news_counter > $max_news_flash_items)
        return;

      echo "          <span class=\"fs\">$date</span> <br>\n";
      echo "          <span class=\"fs\">\n";
      echo "            <a href=\"news.php#item$atom_tag\">$headline</a>\n";
      echo "          </span><p>\n";
   }
   else
   {  
      echo "    <table width=\"90%\">\n";
      echo "      <tr>\n";
      echo "        <td><a name=\"item$atom_tag\"></a><b>${headline}</b></td>\n";
      echo "        <td align=\"right\">$date</td>\n";
      echo "      </tr>\n";
      echo "    </table>\n";
?>
    <table width="90%" cellpadding="0" cellspacing="0">
       <tr>
         <td class="hsep"></td>
       </tr>
    </table>
    <table width="90%" cellpadding="0" cellspacing="0">
       <tr style="height: 5px; width: 100%;">
         <td></td>
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
  
<!-- Main body (Navigation, actual page content, optional news column) --> 
<table width="100%" cellspacing=0>
 <tr>
  <!-- Navigation -->
  <td style="background-color:#f0f0f0; vertical-align:top; width:20%;">
    <table width="100%" cellpadding="10">
      <tr>
        <td>
<?php
echo "         <span class=\"fs\"><b>$trans_contents</b></span>\n";
?>
          <table width="100%" cellpadding="0" cellspacing="0">
	    <tr>
	      <td class="hsep"></td>
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

  <!-- Actual page contents -->
<?php
echo "  <td style=\"vertical-align:top; width:$body_width;\" rowspan=\"2\">\n";

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
   global $trans_news;
   global $trans_fdl;
   global $trans_hosting;
   global $news_flash;
   global $news_counter;
   global $script_lang;

# Close the body table

  echo "  </td> <!-- end of page contents -->\n";

# Insert news flash

  if(!strcmp($script_file, "index.php"))
  {
?>

  <!-- news flash column -->
  <td></td>
  <td style="background-color:#e0e0ff; width:20%;" valign="top">
    <table width="100%" cellpadding="10"><tr><td>
<?php
  echo "      <span class=\"fs\"><b>$trans_news</b></span>\n";
  echo "      <a href=\"http://dvdisaster.net/$script_lang/feed/atom.xml\"><img src=\"../images/atom16.png\" alt=\"Subscribe to dvdisaster news feed\" class=\"noborder\"></a>\n";
?>
      <table width="100%" cellpadding="0" cellspacing="0">
         <tr>
	   <td class="hsep"></td>
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
  </td> <!-- end of news flash column-->
<?php
  }
# Reference to our hoster
/*
 </tr>
 <tr valign="bottom">
   <td style="background-color:#f0f0f0;" align="center">
 <?php echo "     <span class=\"fxs\">$trans_hosting</span><br>\n"; ?>
      <img src="../images/mokelbude.png"
           width="125" height="37" border="0" alt="insider">
   </td>
   <td></td>
   <td></td>
*/
?>

 </tr>

<!--
 <tr valign="bottom">
   <td style="background-color:#f0f0f0;">
<?php 
   $old_lang = strcmp($script_lang, "ru") ? $script_lang : "en";
   echo "      <table cellpadding=\"10\"><tr><td><a href=\"http://dvdisaster.net/legacy/$old_lang/index.html\"><span class=\"fs\">$trans_old_version</span></a></td></tr></table>\n"; 
?>
   </td>
   <td></td>
   <td></td>
 </tr>
-->
</table> <!-- end of main body table -->

<?php
# Create the footer
?>

<!-- Page footer -->
<table width="100%" cellpadding="0" border="0">
 <tr><td colspan="2" style="width:100%; height:10px;"></td>
 </tr>
 <tr><td colspan="2" class="hsep"></td>
 </tr>
 <tr>
  <td align="center">
   <span class="fs">
<?php
   echo "     <i> $trans_copyright<br>\n";
   echo "         $trans_fdl\n";
?>
    </i>
   </span>
  </td>
 </tr>
 <tr><td colspan="2" class="hsep"></td>
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
   echo "<table width=\"100%\" style=\"background-color:#f0f0f0;\">\n";
   echo "<tr><td><span class=\"fl\"><b>$headline</b></span></td>\n";
   echo "<td rowspan=\"2\" align=\"right\"><img src=\"$image\" alt=\"\"></td></tr>\n";
   echo "<tr><td><i>$subtitle</i></td></tr>\n";
   echo "</table><p>\n";
}
?>
