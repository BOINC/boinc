<?php

function csort($array, $column,$column2){
    $i=0;
    for($i=0; $i<count($array); $i++){
        $sortarr[]=$array[$i][$column];
        $tortarr[]=$array[$i][$column2];
    }
    array_multisort($sortarr, $tortarr, $array);
    return($array);
}

function getContentArray($content_header){
    $data=$content_header;
    $entries=explode("|||",trim($data));
    //echo "h".$content_header;
    for ($i=0; $i<sizeof($entries)-1;$i++){
        list($contents[$i]["filename"],$contents[$i]["description"], $contents[$i]["origin"],$contents[$i]["timestamp"],$contents[$i]["hash"], $contents[$i]["size"], $contents[$i]["position"])=explode("|+|", $entries[$i]);
        list($contents[$i]["name"],$contents[$i]["short_description"],$longdesc,$inst,$contents[$i]["platform"],$contents[$i]["platform_add"],$contents[$i]["user"],$contents[$i]["mime_type"],$contents[$i]["real_origin"],$contents[$i]["website"],$contents[$i]["version"])=explode("[[@",$contents[$i]["description"]);
    }
    return $contents;
}


function BOINC_download_network_platform_downloads($realplatform, $contents) {
    $contents=csort($contents, "name", "timestamp");
    $filenumber=-1;
    for ($i=0;$i<sizeof($contents);$i++){
        if (
        (($contents[$i]["platform"]=="4" or $contents[$i]["platform"]=="2" or $contents[$i]["platform"]=="7" or $contents[$i]["platform"]=="9") and $realplatform==2)
        or (($contents[$i]["platform"]=="1" or $contents[$i]["platform"]=="4" or $contents[$i]["platform"]=="6" or $contents[$i]["platform"]=="9") and $realplatform==1)
        or (($contents[$i]["platform"]=="3" or $contents[$i]["platform"]=="7" or $contents[$i]["platform"]=="6" or $contents[$i]["platform"]=="9") and $realplatform==3)
        or ($contents[$i]["platform"]=="8" and $realplatform==8))
        {
            if ($contens[$i]["platform"]!=100 and $contents[$i+1]["filename"]!=$contents[$i]["filename"]){
              row3("<a href='http://www.boinc.dk/download.php?file=".$contents[$i]["filename"]."'>".$contents[$i]["name"]."</a>",$contents[$i]["version"],$contents[$i]["short_description"]);
            }
            //echo $contents[$i]["platform"];
        }
    }
}

function refresh_file() {
    $master_server="http://www.boinc.dk/download.php?command=get_header_file";
    $download_temp_file="../inc/download_network.tmp";
    $mirror_file="../inc/download_network.fil";
    if (@$fhandle = fopen($master_server, "rb")){
        $fhandle2 = fopen($download_temp_file, "wb");
        if (!$fhandle2) {
            echo "CAN'T OPEN TEMP FILE\n";
            return;
        }
        $data = "";
        do {
            $io = fread($fhandle, 8192);
            if (strlen($io) == 0) {
                break;
            }
            fwrite($fhandle2, $io);
        } while (true);
        fclose($fhandle);
        fclose($fhandle2);
        unlink($mirror_file);
        rename($download_temp_file,$mirror_file);
    } else {
       echo "<br>Refreshing the application list failed.  Information here is currently not up to date.<br>";
    }
}

function BOINC_download_network_print_download_links() {
    $mirror_file_header_size=512000;
    $mirror_file="../inc/download_network.fil";
    $platforms[1]["nr"]=1;//Windows
    $platforms[1]["name"]="Windows";
    $platforms[2]["nr"]=2;//UNix
    $platforms[2]["name"]="Linux/Unix";
    $platforms[3]["nr"]=3;//Mac
    $platforms[3]["name"]="Mac";
    $platforms[4]["nr"]=8;//Web
    $platforms[4]["name"]="Web application";

    if (!$fhandle=fopen($mirror_file,"r")){
        refresh_file();
        if (!$fhandle=fopen($mirror_file,"r")){
            return;
        }
    }
    $header=fread($fhandle,$mirror_file_header_size);
    $contents=getContentArray($header);
//    print_r($contents);
    fclose($fhandle);
    $i=0;
    do {
        $i++;
        echo "<h2>".$platforms[$i]["name"]."</h2>";
        start_table();
        echo "<tr><th>Application<br><font size=-2>click to download</font>
            </th><th>Version</th><th>Description</th></tr>\n
        ";
        BOINC_download_network_platform_downloads($platforms[$i]["nr"],$contents);
        echo "</table>\n";
    } while ($i<4);
    flush();
    if (filemtime($mirror_file)<time()-(24*3600)){
        refresh_file();
    }
}

?>
