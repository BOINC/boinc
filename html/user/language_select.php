<?php
require_once("../inc/util.inc");
require_once("../inc/translation.inc");
$imgdir = "img/flags/";


$languages = getSupportedLanguages();

if (get_str("set_lang", true)){
    if (!in_array(get_str("set_lang"), $languages) && get_str("set_lang")!="auto"){
        echo "You must select a supported language";
        exit;
    } else {
        setcookie('lang', get_str("set_lang"), time()+3600*24*365);
        header("Location: index.php");
        flush();
        exit;
    }
}

page_head("Language selection");

echo "
    <p>
    This website is available in the following languages.
    Normally the choice of language
    is determined by your web browser's language settings.
    You can override this by clicking on one of the links.
    This will send your browser a cookie which
    stores your language selection on your computer.
    If you experience any problems please check that your browser accepts 
    cookies from our domain.
    </p>
";
echo "<p>The currently selected language is: <em>".tr(LANG_NAME_INTERNATIONAL)."</em> (".tr(LANG_NAME_NATIVE).")</p>";



start_table();
row2("Language symbol", "Language name (click to select)");
for ($i=0; $i<sizeof($languages);$i++){
    $lang_native[$i] = trSpecific(LANG_NAME_NATIVE,$languages[$i]);
    $lang_international[$i] = trSpecific(LANG_NAME_INTERNATIONAL, $languages[$i]);
}

array_multisort($lang_international, $languages, $lang_native);

for ($i=0; $i<sizeof($languages);$i++){
//    if (file_exists($imgdir.$languages[$i].".png")){
//        $im = "<a href=\"language_select.php?set_lang=".$languages[$i]."\"><img src=\"".$imgdir.$languages[$i].".png\" border=0></a>";
//    } else {
//        $im="";
//    }
//    row3($im,
    row2(
        "<a href=\"language_select.php?set_lang=".$languages[$i]."\">".$languages[$i]."</a>",
        "<a href=\"language_select.php?set_lang=".$languages[$i]."\">".$lang_international[$i]."</a>"
    );
}
end_table();
$prefs = $HTTP_SERVER_VARS["HTTP_ACCEPT_LANGUAGE"];
echo "
    <p>
    For language selection based on your Web browser preferences,
    <a href=\"language_select.php?set_lang=auto\">click here</a>.
    <p>
    Your current Web browser preferences are: <pre>$prefs</pre>
";
page_tail();
?>
