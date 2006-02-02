<?php

//Scale an image using the most powerful GD software available on the server
//while keeping aspect ratio the same
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
        imageAntiAlias($tempImage, true);
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
        imageAntiAlias($tempImage, true);
        imagecopyresampled($destImage, $tempImage, 0, 0, $offsetx, $offsety, $fw, $fh, $fw, $fh);
    } else {
        $destImage = imageCreate($fw, $fh);
        imagecopyresized($destImage, $tempImage, 0, 0, $offsetx, $offsety, $fw, $fh, $fw, $fh);
    }

    return $destImage; //imageJpeg($destImage, $destfile, $jpegquality);
}

?>
