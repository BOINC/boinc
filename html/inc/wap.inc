<?PHP

// BOINC WAP functions

function wap_begin() {
    header("Content-type: text/vnd.wap.wml");
    header("Expires: Thu, 01 Jan 1970 00:00:00 GMT");

    echo "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>"
     . "<!DOCTYPE wml PUBLIC \"-//WAPFORUM//DTD WML 1.2//EN\" "
     . "\"http://www.wapforum.org/DTD/wml_1.2.xml\">"
     . "<wml><card id=\"cd1\"><p>";
}

function wap_end() {
    echo "</p></card></wml>";
}

function wap_timestamp() {
  return strftime("%d %b %Y %H:%M:%S UTC", mktime());
}
?>
