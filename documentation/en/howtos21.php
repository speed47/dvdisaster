<?php
# dvdisaster: English homepage translation
# Copyright (C) 2004-2009 Carsten Gnörlich
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

require("../include/dvdisaster.php");
require("../include/footnote.php");
$script_path = current(get_included_files());
$script_name = basename($script_path, ".php");
begin_page();
$answer=$_GET["answer"];
howto_headline("Creating error correction data", "Decision help", "images/create-icon.png");
?>

<!--- Insert actual page content below --->

<h3>Decision help</h3>

Error correction data can be either created in form of a separate error
correction file or it can be placed directly onto the medium.
Click on the answers to the following questions to find out which
method is most appropriate for you.<p>

<i>Do you need error correction data for a medium which already exists?</i>
<ul>
<?php
if($answer <= 1) $mode="active"; else $mode="passive";
  echo "<li><a href=\"howtos21.php?answer=1\" class=\"$mode\">Yes, the medium has already been written.</a></li>\n";;
if($answer != 1 || $answer >= 2) $mode="active"; else $mode="passive";
  echo "<li><a href=\"howtos21.php?answer=2\" class=\"$mode\">No, but I am planning to write the medium now.</a></li>\n";
echo "</ul>\n";

if($answer == 1) 
{  echo "You need to create an <a href=\"howtos22.php\">error correction file</a>\n";
   echo "because an already existing medium can not be augmented with error correction data.\n";
}

if($answer >= 2)
{  echo "<i>How many free space is left on the medium?</i>\n";
   echo "<ul>\n";
   if($answer >= 2 && $answer != 4) $mode="active"; else $mode="passive";
      echo "<li><a href=\"howtos21.php?answer=3\" class=\"$mode\">The medium has more then 20% free space left.</a></li>\n";
   if($answer >= 2 && $answer != 3) $mode="active"; else $mode="passive";
      echo "<li><a href=\"howtos21.php?answer=4\" class=\"$mode\">The medium is full or nearly full (less then 20% free)</a></li>\n";
   echo "</ul>\n";

   if($answer == 3)
   {  echo "You can put the error correction data <a href=\"howtos33.php\">directly onto the medium</a>.\n";
      echo "To do so you must create an ISO image first and then augment it\n";
      echo "with error correction data before you write it to the medium.\n";
   }
   if($answer == 4)
   {  echo "There is not enough space left on the medium for storing the error correction data.\n";
      echo "You must create a separate <a href=\"howtos22.php\">error correction file</a>.\n";
   }
}
?>

<h3>More information on keeping error correction data</h3>

dvdisaster helps protecting your media from data loss by 
forehanded<sup>*)</sup> creation of error correction data.
Error correction data must be treated like normal backup data, e.g.
you need to keep it available during the whole lifetime of the
respective medium.<p>

The easiest way is storing the error correction data on the medium
you want to protect. But this is only possible if the medium has not yet
been written: To employ this method you need to create an ISO image first,
then augment this image with error correction data, and finally write
the augmented image to the medium.<p>

If the medium has already been written, or insufficient space is left for
augmenting the image, you still can create error correction data in form
of a free-standing error correction file.
This file must then be stored somewhere else, e.g. you need to take additional
provisions to <a href="howtos24.php">archive</a> your error 
correction files.<p>

More information about the pro and con of these methods
can be found in the <a href="http://dvdisaster.net/legacy/en/background30.html">old documentation</a>. 

<pre> </pre>

<!--- do not change below --->

<?php
footnote("*","footnote","Let's repeat again for clarity: Error correction data must be created before the medium becomes defective. It is not possible to create error correction data from defective media; in that case unreadable sectors can not be recovered.");

# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
