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
begin_page();
?>

<!--- Insert actual page content below --->

<h3><a name="top">General questions and answers</a></h3>

<a href="#pronounce">1.1 How is "dvdisaster" pronounced?</a><p>
<a href="#pipo">1.2 What are quality scans and why don't you support more?</a><p>
<a href="#compat">1.3 Is dvdisaster compatible with future releases?</a><p>
<a href="#eccpos">1.4 Augmented images have the error correction data appended at the end of the medium. Isn't that a bad choice?</a><p>
<a href="#recovery">1.5 What's the difference between image based and file based data recovery?</a><p>

<hr><p>

<b><a name="pronounce">1.1 How is "dvdisaster" pronounced?</a></b><p>
Since the word stems from the english language, simply spell "dv" before
saying "disaster". Perhaps "dee-vee-disaster" is a suitable
phonetic circumscription.
<div align=right><a href="#top">&uarr;</a></div>

<b><a name="pipo">1.2 What are quality scans and why don't you support more?</a></b><p>
Optical media have a built-in error correction which is similar to 
the method used in dvdisaster. 
Some drives can report the number of errors corrected
while reading a medium. This provides a rough estimate 
of the writing and media qualities.<p>

Since dvdisaster is free software, it can only include code and information
which can be redistributed freely. This is currently true 
for C2 <a href="howtos10.php">scanning</a> of CD media, 
which is officially standardized and has free documentation available.<p>

On the other hand, DVD quality scans ("PI/PO scans") are not standardized. 
Those drive vendors who support it are using proprietary programming
interfaces. The respective specifications seem not to be available 
for use in free software. So we must patiently wait until manufacturers 
understand that having free software available for a drive 
will sell more drives. <p>

<div align=right><a href="#top">&uarr;</a></div>

<b><a name="compat">1.3 Is dvdisaster compatible with future releases?</a></b><p>
Yes, dvdisaster files are intended for an archival time of many years.
When upgrading to a newer version of dvdisaster you can continue using
images and error correction data created from previous versions.
There is <i>no</i> need to recreate them again.

<div align=right><a href="#top">&uarr;</a></div><p>

<b><a name="eccpos">1.4 Augmented images have the error correction data appended at the end of the medium. Isn't that a bad choice?</a></b><p>
No. First a bit of terminology:
If we augment 80 bytes of user data with 20 bytes of error correction
data, we get an "ecc block" comprised of 100 bytes.
Now take the following into consideration about the ecc block:

<ol>
<li>The position of the error correction data within the ecc block 
does not matter.<p>
The RS decoder does not differentiate between
user data and error correction data. In the view of the RS decoder
our ecc block is a sequence of 100 bytes from which an arbitrary
subset of 20 bytes can be recovered. It can recover
the first 20 bytes, the last 20 bytes, or any 
combination from within as long as the remaining 80 bytes
are still intact. 
From this it follows that the position of the
ecc data within the ecc block does not matter; whether it is appended
at one end of the user data or is interleaved with it
has no influence on the error correcting capability.</li>

<li>Properly distributing the ecc block offsets influence of bad media spots.<p>
Optical media have a higher probability of failing in the outer area; 
for technical reasons this is
the only place where the error correction data can be stored. 
However this effect is offset
by distributing the ecc block content over the medium. Let's assume that our
medium is filled 80% with user data, leaving the remaining 20% free for
error correction data. Now consider the 100 byte ecc block again.
We need to pick 80 bytes from the user data for it and require 20
additional byte positions in the error correction data area.
Even under these constraints it is possible to evenly distribute the
100 bytes over the medium, from the
inside to the outside, each having a maximum distance to its neighbors.
Together with point (1), this negates the influence of bad spots on the 
medium. Symmetry implies that for each error correction byte
stored in the (bad) outer region there will 
be a user data byte located in the (good) inner medium region.
<p>
(If you do not already see the point, imagine putting the ecc data into
the inner medium region and the user data in the outer region. Consider
point (1) again to see that nothing changes with respect to the error
correction.) 
</li>
</ol>
<div align=right><a href="#top">&uarr;</a></div><p>


<b><a name="recovery">1.5 What's the difference between image based and file based data recovery?</a></b><p>

Optical media are comprised of 2048 byte-wide sectors. Most of those sectors
are used to store file data, but some of them hold so-called "meta data",
e.g. information on directory folders.<br>
In figure 1.5.1 (below) there is
a directory "Pics" holding three files "forest.jpg", "rock.jpg" and
"protect.par"<a href="#note1"><sup>1)</sup></a>. 
Note how these files are mapped onto physical sectors
(green/blue squares) on the medium, 
and that an additional meta data sector  (red square) is needed
for storing the "Pics" directory structure.<p>

<table width="100%"><tr><td align="center"><img src="images/metadata1.png"></td></tr></table><p>

<b>Shortcoming of file based recovery on optical media.</b><br>
Now let's assume that we are working with file based error correction.
The file "protect.par" holds error correction information which can be used
to recover unreadable sectors within the files "forest.jpg" and "rock.jpg".
This will only work as long as we need to recover sectors which are part 
of a file. But if meta data sectors become unreadable, the 
file based protection will collapse. Consider figure 1.5.2. When the
red directory sector becomes unreadable, not only the directory "Pics"
but also all files under "Pics" become inaccessible.
This is due to the logical structure of the ISO/UDF file system, 
as there is no way to tell how the green and blue sectors relate 
to files anymore when the directory is lost. So we have
a complete data loss although all sectors comprising the files
are still physically readable.<p>

<table width="100%"><tr><td align="center"><img src="images/metadata2.png"></td></tr></table><p>

<p>Please note that moving "protect.par" to a separate medium does not
rectify the problem - the directory block is still not recoverable as
it is not protected by the error correction data in "protect.par".<p>

<b>Advantages of image level recovery on optical media.</b><br>

dvdisaster applies an image level approach to error recovery.
The medium is read and processed as an ISO image. 
 The ISO image contains a sequence of all sectors found on
the medium, including those which are meta data for the file system.
Since the dvdisaster error correction data protects <i>all sectors</i> in the
ISO image, file contents as well as meta data sectors (e.g. directories)
can be restored. See fig. 1.5.3 for the different range of protection.

<table width="100%"><tr><td align="center"><img src="images/metadata3.png"></td></tr></table><p>

In addition, neither reading the damaged ISO image nor applying the
error correction requires any information from the file system 
contained on the medium. As long as the
drive is still able to recognize the medium, dvdisaster will be able
to recover the still readable sectors from it. Therefore
there are no "single sectors of failure" as in the file based approach.

<table width="30%" cellpadding="0">
<tr bgcolor="#000000"><td><img width=1 height=1 alt=""></td></tr>
</table>
<a name="note1"><sup>1)</sup>
No offense intended against the PAR/PAR2 project. 
Carsten is just confident that file based protection does not work 
on optical media :-)</a>


<div align=right><a href="#top">&uarr;</a></div><p>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
