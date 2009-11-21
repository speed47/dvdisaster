<?php

#  dvdisaster homepage: Footnote functions
#  Copyright (C) 2007-2009 Carsten Gnörlich
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

function footnote($symbol, $name, $content)
{
   echo "<table width=\"30%\" cellpadding=\"0\"><tr bgcolor=\"#000000\"><td><img width=1 height=1 alt=\"\"></td></tr></table>\n";
   echo "<a name=\"$name\"><sup>$symbol)</sup>$content</a>\n";
}
?>
