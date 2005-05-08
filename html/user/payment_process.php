<?php
/*
 * process.php
 *
 * PHP Toolkit for PayPal v0.50
 * http://www.paypal.com/pdn
 *
 * Copyright (c) 2004 PayPal Inc
 *
 * Released under Common Public License 1.0
 * http://opensource.org/licenses/cpl.php
 *
*/

//Configuration File
include_once('../inc/paypal_config.inc'); 

//Global Configuration File
include_once('../inc/paypal_global_config.inc');
?> 

<html>
<head><title>Processing transaction...</title></head>
<body onLoad="document.paypal_form.submit();">
<form method="post" name="paypal_form" action="<?=$paypal[url]?>">

<?php 
//show paypal hidden variables

showVariables(); 

?> 

<center><font face="Verdana, Arial, Helvetica, sans-serif" size="2" color="333333">Processing Transaction . . . </font></center>

</form>
</body>   
</html>
