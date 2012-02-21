<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// Scale an image using the most powerful GD software available on the server
// while keeping aspect ratio the same
//
function intelligently_scale_image($sourcefile, $fw, $fh) {
    $gd_info = gd_info();
    $newGD = (strstr($gd_info["GD Version"], "2.0")!="");

    list($ow, $oh, $from_type) = getimagesize($sourcefile);
    switch($from_type) {
    case 1: // GIF
        $srcImage = imageCreateFromGif($sourcefile);
        break;
    case 2: // JPG
        $srcImage = imageCreateFromJpeg($sourcefile);
        break;
    case 3: // PNG
        $srcImage = imageCreateFromPng($sourcefile);
        break;
    }

    $tempw = $fw;
    $temph = number_format((($oh*$fw)/$ow), 0);

    if($temph < $fh) {
        $tempw = number_format((($ow*$fh)/$oh), 0);
        $temph = $fh;
    }

    if ($newGD){
        $tempImage = imageCreateTrueColor($tempw, $temph);
        // Seems not to work:
        // imageAntiAlias($tempImage, true);
        imagecopyresampled($tempImage, $srcImage, 0, 0, 0, 0, $tempw, $temph, $ow, $oh);
    } else {
        $tempImage = imageCreate($tempw, $temph);
        imagecopyresized($tempImage, $srcImage, 0, 0, 0, 0, $tempw, $temph, $ow, $oh);
    }


    // Calculate offsets
    if($temph < $fh) {
        $offsety = number_format(($temph/2)-($fh/2), 0);
        $offsetx = 0;
    } else {
        $offsety = 0;
        $offsetx = number_format(($tempw/2)-($fw/2), 0);
    }

    if ($newGD){
        $destImage = imageCreateTrueColor($fw, $fh);
        // Seems not to work:
        // imageAntiAlias($tempImage, true);
        imagecopyresampled($destImage, $tempImage, 0, 0, $offsetx, $offsety, $fw, $fh, $fw, $fh);
    } else {
        $destImage = imageCreate($fw, $fh);
        imagecopyresized($destImage, $tempImage, 0, 0, $offsetx, $offsety, $fw, $fh, $fw, $fh);
    }

    return $destImage; //imageJpeg($destImage, $destfile, $jpegquality);
}

?>
