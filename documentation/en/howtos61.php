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

<h3>The idea behind the error correction</h3>

<table width="100%">
<tr valign="top">
<td><img src="../images/bad-cd.png"></td>
<td> </td>
<td><img src="../images/ecc.png"></td>
<td> &nbsp; </td>
<td rowspan="3">
The example from the previous page told us how dvdisaster reconstructs
data by using the still readable parts of the medium together with
the error correction data.<p>

In order to get the most out of dvdisaster a basic understanding 
of the error correction method is helpful. And while we are at it we
can refute a misunderstanding we sometimes hear - the error correction
data is <b>not</b> simply a copy of the last 20% data sectors.
That'd really be a cheap shot ;-)
</td>
</tr>

<tr>
<td align="right">80%<img src="../images/rdiag-arrow.png"></td>
<td> </td>
<td align="left"><img src="../images/ldiag-arrow.png">20%</td>
<td> </td>
</tr>

<tr>
<td> </td>
<td> <img src="../images/good-image.png"></td>
<td> </td>
<td> </td>
</tr>
</table><p>

<b>Example: Anna's desk drawer PIN</b><p>

Anna has got a desk whose drawers can only be opened after entering
the numbers "8 6 2 3" into a code lock. Since the drawers do not contain
any sensitive information she decides to note down the numbers directly
on the desktop:<p>

<img src="../images/ecc-example1.png"><p>

Anna is cautious and expects one of the numbers to become unreadable 
by accidentally pouring ink over it. Therefore she also notes down
the sum of the four numbers (the "+" and "=" signs have only be added for
clarity):<p>

<img src="../images/ecc-example2.png"><p>

After a while one of the numbers indeed gets covered by an ink spot:<p>

<img src="../images/ecc-example3.png"><p>

But this is not a problem as Anna can re-calculate the missing 
number <i>x</i>
by rearranging the still readable parts of the equation:<p>

8 + x + 2 + 3 = 19, hence<p>

x = 19 - 8 - 2 - 3, and therefore x = 6.<p>

It is easily seen that any one of the original five numbers can be
recovered from the remaining four. The example also demonstrates
some important properties of the error correction:
<p>

<table><tr><td><img src="../images/ecc-example4.png"></td><td>&nbsp;&nbsp;</td>
<td valign="top">
For a given set of data (e.g. the numbers "8 6 2 3")
additional error correction data (e.g. the sum "19") can be created
so that a lost datum can be re-calculated from the remaining data.<p>

The same principle is used in dvdisaster; the protected sequence of numbers
is nothing else than the ISO image of a CD, DVD or BD.</td>
</tr></table><p>

The concept of <b>redundancy</b> can be explained as follows:

<ul>
<li>One "error correction number" is calculated for four input numbers.
1 of 4 (or 1/4) relates to a redundancy of 25%.</li>
<li> From one error correction number we can re-calculate exactly one missing
number, or at most 25% of data. 
The redundancy is equivalent to the maximum capacity of the error correction.</li>
<li> Additional storage required for the error correction data is also
determined by the redundancy (25% in the example).</li>
</ul>

dvdisaster uses the term of redundancy accordingly. In addition please
observe that
<ul>
<li>no data can be recovered when the data loss exceeds the redundancy
(the equation in the example can not be solved for two or more unknowns).</li>
<li>the error correction data must be calculated at a point in time
where all data is still present / readable.</li>
</ul><p>

The above shown example does not generalize into an error correction
scheme for recovering more than one missing data value. To do so a more
powerful equation system is needed which can be solved for more than
one missing value. dvdisaster uses a Reed-Solomon code
which does have such properties; however the required math is not taught
in school. Interested readers are therefore referred to the respective
books in coding theory.

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
