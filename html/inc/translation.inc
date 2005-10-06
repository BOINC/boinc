<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

$lang_language_dir = "../languages/";
$lang_translations_dir = "translations/";
$lang_prj_translations_dir = "project_specific_translations/";
$lang_compiled_dir = "compiled/";
$lang_project_default = "en";
$lang_log_level = 0;
$lang_log_file = $lang_language_dir."translator.log";


function getSupportedLanguages(){
    global $lang_language_dir, $lang_compiled_dir;
    //echo $lang_language_dir.$lang_compiled_dir;
    if (is_dir($lang_language_dir.$lang_compiled_dir)) {
        if ($dh = opendir($lang_language_dir.$lang_compiled_dir)) {                        //If dir exists
            while (($file = readdir($dh)) !== false) {
                //read contents
                if ($file==".." or $file=="."){
                } else if (substr($file,-7)==".po.inc"){
                    //only files ending in .po.inc
                    $list[] = substr($file,0,-7);
                }
            }
        }
    } else {echo "uh";}
    //print_r($list);
    return $list;
}

function trSpecific($tokennumber, $language=""){
    global $lang_language_dir, $lang_compiled_dir;
    $language= substr($language,0,5);
    if ($language!="" && file_exists($lang_language_dir.$lang_compiled_dir.$language.".po.inc")){
        //If we have the language, include it
        //echo "[$language]";
        require($lang_language_dir.$lang_compiled_dir.$language.".po.inc");
        return stripslashes($language_lookup_array[$tokennumber]);
    } else {
        return false;
    }
}    

/*************************
 * Parses the language interface file containing the tokens (can parse .po-style files)
 ************************/
function parseLanguageInterface($file){
    $translation_file = file($file);
    for ($i = 0;$i<sizeof($translation_file);$i++){
        $entry = ltrim(trim($translation_file[$i]));
        if (($pos=strpos($entry, "msgid"))!==false){
            //If found msgid entry
            $token=getPOLineContent($entry);
            if (ltrim(trim($token))!=""){
                $interface[]=$token;
            }
        }
    }
    return $interface;
}


/***************************
 * Builds the lookup arrays from the language interface file
 * and the language files found in the $dir directory subtree
 ***************************/
function buildLanguages($langdir,$transdir,$prjtransdir,$compdir){
    global $lang_project_default;

    $actual_compdir = $compdir; // Store the actual compile dir, we want to use a temporary addition
    $randomness = rand(10000,99999)."_"; //Concat some randomness to the temporary dir, we'll rename files later
    $compdir .=$randomness; // This is to ensure that no problems arise even when the filesystem fails to
			    //ensure mutual exclusion for compile timer tests. This way a new file will simply
			    //overwrite the old one instead of garbling it if 2 processes of this compiler runs
			    //concurrently.
    //The reason why we are using filename prefixes here is that on some installations PHP isn't allowed
    //to create and use new directories (due to wrong permissions or SELinux restrictions).

    //Touching compile timer
    $fh=fopen($langdir."last_compile_timer","w");
    fwrite($fh, time());
    fclose($fh);

    //First build the list of defines for the tokens
    //and place it in an include file in the directory
    //

    $interface = parseLanguageInterface($langdir.$transdir.$lang_project_default.".po");
    if (!$fh = fopen($langdir.$compdir."language_interface.inc","w")) {
        language_log("Could not access $langdir $compdir - please check permissions",2);
        exit;
    };
    fwrite($fh, "<?php\n");
    for ($i=0;$i<sizeof($interface);$i++){
        //Translate to PHP
        fwrite($fh, "define(\"".$interface[$i]."\",".$i.");\n");
    }
    fwrite($fh, '?>');
    fclose($fh);
    $real_interface_size = sizeof($interface);
    
    //Now run trough each language and recompile their lookup arrays.
    //
    if (is_dir($langdir.$transdir)) {
        if ($dh = opendir($langdir.$transdir)) {
            //If dir exists
            while (($file = readdir($dh)) !== false) {
                //read contents
                if ($file==".." or $file=="."){
                } else if (substr($file,-3)==".po"){
                    //only files ending in .po
                    language_log("----------------Compiling $file---------------",0);
                    $language = parseLanguage($langdir.$transdir.$file, $interface);
                    if (!$fh = fopen($langdir.$compdir.$file.".inc","w")) {
                        language_log("ERROR: could not access $langdir $compdir - please check permissions",2);
                        exit;
                    };
                    fwrite($fh, "<?php\n");
                    $keys = array_keys($language);
                    for ($i=0;$i<sizeof($interface);$i++){
                        if (isset($keys[$i]) && isset($language[$keys[$i]]) && $language[$keys[$i]]!="") {
                            //Translate to PHP
                            fwrite($fh, "\$language_lookup_array[".$keys[$i]."] = \"".addslashes($language[$keys[$i]])."\";\n");
                        }
                    }
                    fwrite($fh, '?>');
                    fclose($fh);
                } else {
                    //debug("File $file with unknown extension found in $info_dir");
                }
            }
            closedir($dh);
        } else {
            //debug("$info_dir could not be opened - check permissions?");
        }
    } else {
        //debug("$info_dir not found or is not a directory");
    }

    //Do the same again, this time for the project specific language files
    // and instead of creating new compiled files
    // just add to whatever existing ones
    //
    if (is_dir($langdir.$prjtransdir)) {
        $interface = parseLanguageInterface($langdir.$prjtransdir.$lang_project_default.".po");
        if (!$fh = fopen($langdir.$compdir."language_interface.inc","a")) {
            language_log("ERROR: could not access $langdir $compdir - please check permissions",2);
            exit;
        };
        fwrite($fh, "<?php\n");
        for ($i=0;$i<sizeof($interface);$i++){
            fwrite($fh, "define(\"".$interface[$i]."\",".($i+$real_interface_size).");\n");
        }
        fwrite($fh, '?>');
        fclose($fh);
        if ($dh = opendir($langdir.$prjtransdir)) {
            //If dir exists
            while (($file = readdir($dh)) !== false) {
                //read contents
                if ($file==".." or $file=="."){
                } else if (substr($file,-3)==".po"){
                    //only files ending in .po
                    language_log("----------------Compiling project specific $file---------------",0);
                    $language = parseLanguage($langdir.$prjtransdir.$file, $interface);
                    if (!$fh = fopen($langdir.$compdir.$file.".inc","a")) {
                        language_log("ERROR: could not access $langdir $compdir - please check permissions",2);
                        exit;
                    };
                    fwrite($fh, "<?php\n");
                    $keys = array_keys($language);
                    for ($i=0;$i<sizeof($interface);$i++){
                        if ($language[$keys[$i]]!="") {
                            fwrite($fh, "\$language_lookup_array[".($real_interface_size+$keys[$i])."] = \"".addslashes($language[$keys[$i]])."\";\n");                //Translate to PHP
                        }
                    }
                    fwrite($fh, '?>');
                    fclose($fh);
                } else {
                    //debug("File $file with unknown extension found in $info_dir");
                }
            }
            closedir($dh);
        } else {
            //debug("$info_dir could not be opened - check permissions?");
        }
    } else {
        //debug("$info_dir not found or is not a directory");
    }

    //Lastly we will rename the files to their real names (remove the random prefix):
    if ($dh = opendir($langdir.$actual_compdir)) {                        //If dir exists
            while (($file = readdir($dh)) !== false) {
                //read contents
                if ($file==".." or $file=="." or (substr($file,0,strlen($randomness))!=$randomness)){
		    // Do nothing
                } else if (substr($file,-4)==".inc"){
                    //only files ending in .inc
		    rename ($langdir.$actual_compdir.$file, $langdir.$actual_compdir.substr($file,strlen($randomness)));
                }
            }
    } else {
	echo "no compile dir?!";
    }
}


/***************************
 * Have some of the files changed?
 **************************/
function languagesNeedsRebuild($langdir,$transdir,$prjtransdir, $compdir){

    if (!file_exists($langdir.$compdir)){
        //This directory does not exist - try to create one
        mkdir($langdir.$compdir,0733);
        return true;
        //and force an update since we need it
    }

//    return false;                //Uncomment this to speed up things when you know that you will manually recompile the languages when updated...
    
    $last_compile = filemtime($langdir."last_compile_timer");
    //This file gets touched each time a compile finishes

    if (is_dir($langdir.$transdir)) {
        if ($dh = opendir($langdir.$transdir)) {
            //If dir exists
            while (($file = readdir($dh)) !== false) {
                //read contents
                if ($file==".." or $file=="."){
                } else if (substr($file,-3)==".po"){
                    //only files ending in .po
                    if (filemtime($langdir.$transdir.$file)>$last_compile) return true;
                }
            }
            closedir($dh);
        }
    }
    if (is_dir($langdir.$prjtransdir)) {
        if ($dh = opendir($langdir.$prjtransdir)) {
            //If dir exists
            while (($file = readdir($dh)) !== false) {
                //read contents
                if ($file==".." or $file=="."){
                } else if (substr($file,-3)==".po"){
                    //only files ending in .po
                    if (filemtime($langdir.$prjtransdir.$file)>$last_compile) return true;
                }
            }
            closedir($dh);
        }
    }
    return false;
    //All checks say that nothing has changed.
}

/**************************
 * Parses a gettext .po-file into an indexed PHP array, 
 * checking for inconsistencies if needed.
 * The $file is parsed and validated against $interface
 *************************/
function parseLanguage($file, $interface){
    if (sizeof($interface)<1){
        language_log("No interface defined for 'compileLanguages()'",2);
        exit;
    }
    $translation_file = file($file);
    $first_entry = true;
    for ($i = 0;$i<sizeof($translation_file);$i++){
        $entry = ltrim(trim($translation_file[$i]));
        //echo $entry;
        if (($pos=strpos($entry, "msgid"))!==false){
            //If found msgid entry
            if (!$first_entry){
                //If this is not the first, save the previous entry
                //Does token msgid entry exist in interface?
                //If so, add to output
                $id = checkToken($current_token, $interface, $i);
                if ($id!==false){
                    $output[$id]=$current_token_text;
                }
            }
            $current_token = getPOLineContent($entry);
            $current_token_text="";
            $first_entry=false;
            //Now it is no longer the first entry
        } elseif (substr($translation_file[$i],0,1)!="#") {
            //echo "Else";
            $current_token_text.=getPOLineContent($entry);
        }
    }
    $id = checkToken($current_token, $interface, $i);
    if ($id!==false){
        $output[$id]=$current_token_text;
    }
    checkMissingTokens($output, $interface);
    return $output;
}

/*********************
 * Checks if token is in interface.
 * Displays error if not
 **********************/
function checkToken($current_token, $interface, $line){
    $id = array_search($current_token,$interface);
    if ($id==false){
        language_log("Above line ".$line.": Language file $file has a token (".$current_token.") that is not defined in the interface.",1);
        return false;
    } else {
        return $id;
    }
}

/*****************
 * This function prints any missing tokens as errors
 *****************/
function checkMissingTokens($language_array, $interface){
    $diff = array_values(array_diff( array_keys($interface),array_keys($language_array)));
    for ($i=0; $i<sizeof($diff); $i++){
        language_log("Language file is missing token (".$interface[$diff[$i]].")",1);
    }
}

/***********************
 * Returns the contents of a line (ie removes "" from start and end)
 *********************/
function getPOLineContent($line){
    $start = strpos($line, '"')+1;
    $stop = strrpos($line, '"');
    return substr($line, $start, $stop-$start);
}

/************************
 * Translate token
 * Use the token name as a constant - like echo tr(MENU_ABOUT);
 ************************/
function tr($tokennumber){
    global $language_lookup_array;
    if (strval(intval($tokennumber))!=$tokennumber){
        echo "token $tokennumber missing from language interface.";
        language_log("token $tokennumber missing from language interface.");
    }
    
    if (array_key_exists($tokennumber,$language_lookup_array)){
        //If language has got the token
        //If found in client language, return that
        return stripslashes($language_lookup_array[$tokennumber]);        
    }
    return "[MISSING TEXT]";                                                //If not found there display "MISSING TEXT"
}

function language_log($message, $loglevel=0){
    global $lang_log_level, $lang_log_file;
    if ($loglevel==0) $msg = "[ Debug    ]";
    if ($loglevel==1) $msg = "[ Warning  ]";
    if ($loglevel==2) $msg = "[ CRITICAL ]";
    
    if ($loglevel>=$lang_log_level){
        $fh = fopen($lang_log_file,"a");
        fwrite($fh, date("Y-m-d H:i:s",time())." ".$msg." ".$message."\n");
        fclose($fh);
    }
}




if (languagesNeedsRebuild($lang_language_dir, $lang_translations_dir, $lang_prj_translations_dir, $lang_compiled_dir) || get_str("compile_languages",true)){
    buildLanguages($lang_language_dir,$lang_translations_dir,$lang_prj_translations_dir, $lang_compiled_dir);
}

// Define some variables (from constants) that the language files can use in the translation:
$PROJECT = PROJECT;


if (file_exists($lang_language_dir.$lang_compiled_dir."language_interface.inc")){
    require_once($lang_language_dir.$lang_compiled_dir."language_interface.inc");

    //Make a list of languages which the user prefers (by looking at cookies and browser settings)
    //cookies have highest priority.

    if (isset($_COOKIE['lang'])){
        $language_string = $_COOKIE['lang'].",";
    } else {
        $language_string = '';
    }
    if (isset($HTTP_SERVER_VARS["HTTP_ACCEPT_LANGUAGE"])) {
        $language_string .= strtolower($HTTP_SERVER_VARS["HTTP_ACCEPT_LANGUAGE"]);
    }

    //Find out which language to use by iterating trough list
    //The list is commaseperated, so split it into an array of the following type:
    //        Array ( 
    //    [0] => da 
    //    [1] => en-us;q=0.7 
    //    [2] => en;q=0.3 
    //  )

    $client_languages=explode(",",$language_string);

    //Now a language is either defined as primary-secondary or merely primary
    //it can also have a quality attribute set - which orders the languages in a user
    //preffered ordering. Since this is usally the same order as the array indices
    //we just ignore this attribute. (TODO: don't ignore this attribute)
    //A missing quality attribute means q=1

    require_once($lang_language_dir.$lang_compiled_dir.$lang_project_default.".po.inc");
    //Always include the project default as fallback
    for ($i=sizeof($client_languages);$i>=0;$i--){
        //Now for each language that the client requests
        if ((strlen($client_languages[$i])>2) && (substr($client_languages[$i],2,1)=="_" || substr($client_languages[$i],2,1)=="-")){
            //If this is defined as primary-secondary
            $language = substr(
                $client_languages[$i],0,2)."_".strtoupper(substr($client_languages[$i],3,2)
            );
            //represent it as xx_YY
        } else {
            $language = substr($client_languages[$i],0,2);
            //else just use xx
        }
        if (file_exists($lang_language_dir.$lang_compiled_dir.$language.".po.inc")) {
            //If we have the language, include it
            //echo "[$language]";
            require($lang_language_dir.$lang_compiled_dir.$language.".po.inc");
        }
    }

} else {
    language_log("Could not load language interface.",2);
    echo "ERROR: Could not load language interface. This is a fatal error, exitting.";
    flush;
    exit;
};

?>
