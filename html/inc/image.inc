<?php

function intelligently_scale_image($sourcefile, $fw, $fh) {

	//In order to use the default built-in GD library all imageCreateTrueColor
	//has been commented out.
	//If you whish to enable the advanced GD features (resamples copy for instance)
	//simply switch back to the commented functions.
	
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

//  $tempImage = imageCreateTrueColor($tempw, $temph);
//  imageAntiAlias($tempImage, true);
    $tempImage = imageCreate($tempw, $temph);

//  imagecopyresampled($tempImage, $srcImage, 0, 0, 0, 0, $tempw, $temph, $ow, $oh);
    imagecopyresized($tempImage, $srcImage, 0, 0, 0, 0, $tempw, $temph, $ow, $oh);


    // Calculate offsets
    if($temph < $fh) {
        $offsety = number_format(($temph/2)-($fh/2), 0);
        $offsetx = 0;
    } else {
        $offsety = 0;
        $offsetx = number_format(($tempw/2)-($fw/2), 0);
    }

//  $destImage = imageCreateTrueColor($fw, $fh);
//  imagecopyresampled($destImage, $tempImage, 0, 0, $offsetx, $offsety, $fw, $fh, $fw, $fh);

    $destImage = imageCreate($fw, $fh);
    imagecopyresized($destImage, $tempImage, 0, 0, $offsetx, $offsety, $fw, $fh, $fw, $fh);

    return $destImage; //imageJpeg($destImage, $destfile, $jpegquality);
}

?>
