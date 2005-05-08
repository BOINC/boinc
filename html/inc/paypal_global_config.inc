<?php
/*
 * global_config.inc.php
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

//create variable names to perform additional order processing

function create_local_variables() { 

$array_name[business]="$_POST[business]"; 
$array_name[receiver_email]="$_POST[receiver_email]"; 
$array_name[receiver_id]="$_POST[receiver_id]"; 
$array_name[item_name]="$_POST[item_name]"; 
$array_name[item_number]="$_POST[item_number]"; 
$array_name[quantity]="$_POST[quantity]"; 
$array_name[invoice]="$_POST[invoice]"; 
$array_name[custom]="$_POST[custom]"; 
$array_name[memo]="$_POST[memo]"; 
$array_name[tax]="$_POST[tax]"; 
$array_name[option_name1]="$_POST[option_name1]"; 
$array_name[option_selection1]="$_POST[option_selection1]"; 
$array_name[option_name2]="$_POST[option_name2]"; 
$array_name[option_selection2]="$_POST[option_selection2]"; 
$array_name[option_name3]="$_POST[option_name3]"; 
$array_name[option_selection3]="$_POST[option_selection3]"; 
$array_name[num_cart_items]="$_POST[num_cart_items]"; 
$array_name[mc_gross]="$_POST[mc_gross]"; 
$array_name[mc_fee]="$_POST[mc_fee]"; 
$array_name[mc_currency]="$_POST[mc_currency]"; 
$array_name[settle_amount]="$_POST[settle_amount]"; 
$array_name[settle_currency]="$_POST[settle_currency]"; 
$array_name[exchange_rate]="$_POST[exchange_rate]"; 
$array_name[payment_gross]="$_POST[payment_gross]"; 
$array_name[payment_fee]="$_POST[payment_fee]"; 
$array_name[payment_status]="$_POST[payment_status]"; 
$array_name[pending_reason]="$_POST[pending_reason]"; 
$array_name[reason_code]="$_POST[reason_code]"; 
$array_name[payment_date]="$_POST[payment_date]"; 
$array_name[txn_id]="$_POST[txn_id]"; 
$array_name[txn_type]="$_POST[txn_type]"; 
$array_name[payment_type]="$_POST[payment_type]"; 
$array_name[for_auction]="$_POST[for_auction]"; 
$array_name[auction_buyer_id]="$_POST[auction_buyer_id]"; 
$array_name[auction_closing_date]="$_POST[auction_closing_date]"; 
$array_name[auction_multi_item]="$_POST[auction_multi_item]"; 
$array_name[first_name]="$_POST[first_name]"; 
$array_name[last_name]="$_POST[last_name]"; 
$array_name[payer_business_name]="$_POST[payer_business_name]"; 
$array_name[address_name]="$_POST[address_name]"; 
$array_name[address_street]="$_POST[address_street]"; 
$array_name[address_city]="$_POST[address_city]"; 
$array_name[address_state]="$_POST[address_state]"; 
$array_name[address_zip]="$_POST[address_zip]"; 
$array_name[address_country]="$_POST[address_country]"; 
$array_name[address_status]="$_POST[address_status]"; 
$array_name[payer_email]="$_POST[payer_email]"; 
$array_name[payer_id]="$_POST[payer_id]"; 
$array_name[payer_status]="$_POST[payer_status]"; 
$array_name[notify_version]="$_POST[notify_version]"; 
$array_name[verify_sign]="$_POST[verify_sign]"; 
 
return $array_name; 

}

//this function creates a comma separated value file from an array. 

function create_csv_file($file,$data) {

//check for array
if(is_array($data)) { $post_values=array_values($data); 

//build csv data
foreach($post_values as $i) { $csv.="\"$i\","; }

//remove the last comma from string
$csv=substr($csv,0,-1); 

//check for existence of file
if(file_exists($file) && is_writeable($file)) { $mode="a"; } else { $mode="w"; }

//create file pointer
$fp=@fopen($file,$mode);
 
//write to file
fwrite($fp,$csv . "\n"); 

//close file pointer
fclose($fp); 

return true; 

} 

else { return false; } 

}

//post transaction data using curl

function curlPost($url,$data)  {

global $paypal; 

//build post string 

foreach($data as $i=>$v) { 
$postdata.= $i . "=" . urlencode($v) . "&"; 
}

$postdata.="cmd=_notify-validate";

//execute curl on the command line

exec("$paypal[curl_location] -d \"$postdata\" $url", $info);

$info=implode(",",$info); 

return $info; 

}

//posts transaction data using libCurl 

function libCurlPost($url,$data)  {

//build post string 

foreach($data as $i=>$v) { 

$postdata.= $i . "=" . urlencode($v) . "&"; 

}

$postdata.="cmd=_notify-validate";

$ch=curl_init(); 

curl_setopt($ch,CURLOPT_SSL_VERIFYPEER,FALSE);
curl_setopt($ch,CURLOPT_URL,$url); 
curl_setopt($ch,CURLOPT_POST,1); 
curl_setopt($ch,CURLOPT_POSTFIELDS,$postdata); 

//Start ob to prevent curl_exec from displaying stuff. 
ob_start(); 
curl_exec($ch);

//Get contents of output buffer 
$info=ob_get_contents(); 
curl_close($ch);

//End ob and erase contents.  
ob_end_clean(); 

return $info; 

}

//posts transaction data using fsockopen. 
function fsockPost($url,$data) { 

//Parse url 
$web=parse_url($url); 

//build post string 
foreach($data as $i=>$v) { 
$postdata.= $i . "=" . urlencode($v) . "&"; 
}

$postdata.="cmd=_notify-validate";

//Set the port number
if($web[scheme] == "https") { $web[port]="443";  $ssl="ssl://"; } else { $web[port]="80"; }  

//Create paypal connection
$fp=@fsockopen($ssl . $web[host],$web[port],$errnum,$errstr,30); 

//Error checking
if(!$fp) { echo "$errnum: $errstr"; } 
 
//Post Data
else { 
 
  fputs($fp, "POST $web[path] HTTP/1.1\r\n"); 
  fputs($fp, "Host: $web[host]\r\n"); 
  fputs($fp, "Content-type: application/x-www-form-urlencoded\r\n"); 
  fputs($fp, "Content-length: ".strlen($postdata)."\r\n"); 
  fputs($fp, "Connection: close\r\n\r\n"); 
  fputs($fp, $postdata . "\r\n\r\n"); 

//loop through the response from the server 
while(!feof($fp)) { $info[]=@fgets($fp, 1024); } 

//close fp - we are done with it 
fclose($fp); 

//break up results into a string
$info=implode(",",$info); 

}

return $info; 

   } 

//Display Paypal Hidden Variables

function showVariables() {

global $paypal; 

?> 

<!-- PayPal Configuration --> 
<input type="hidden" name="business" value="<?=$paypal[business]?>"> 
<input type="hidden" name="cmd" value="<?=$paypal[cmd]?>"> 
<input type="hidden" name="image_url" value="<? echo "$paypal[site_url]$paypal[image_url]"; ?>">
<input type="hidden" name="return" value="<? echo "$paypal[site_url]$paypal[success_url]"; ?>">
<input type="hidden" name="cancel_return" value="<? echo "$paypal[site_url]$paypal[cancel_url]"; ?>">
<input type="hidden" name="notify_url" value="<? echo "$paypal[site_url]$paypal[notify_url]"; ?>">
<input type="hidden" name="rm" value="<?=$paypal[return_method]?>">
<input type="hidden" name="currency_code" value="<?=$paypal[currency_code]?>">
<input type="hidden" name="lc" value="<?=$paypal[lc]?>">
<input type="hidden" name="bn" value="<?=$paypal[bn]?>">
<input type="hidden" name="cbt" value="<?=$paypal[continue_button_text]?>">

<!-- Payment Page Information --> 
<input type="hidden" name="no_shipping" value="<?=$paypal[display_shipping_address]?>">
<input type="hidden" name="no_note" value="<?=$paypal[display_comment]?>">
<input type="hidden" name="cn" value="<?=$paypal[comment_header]?>"> 
<input type="hidden" name="cs" value="<?=$paypal[background_color]?>">

<!-- Product Information --> 
<input type="hidden" name="item_name" value="<?=$paypal[item_name]?>">
<input type="hidden" name="amount" value="<?=$paypal[amount]?>">
<input type="hidden" name="quantity" value="<?=$paypal[quantity]?>"> 
<input type="hidden" name="item_number" value="<?=$paypal[item_number]?>">
<input type="hidden" name="undefined_quantity" value="<?=$paypal[edit_quantity]?>">
<input type="hidden" name="on0" value="<?=$paypal[on0]?>">
<input type="hidden" name="os0" value="<?=$paypal[os0]?>">
<input type="hidden" name="on1" value="<?=$paypal[on1]?>">
<input type="hidden" name="os1" value="<?=$paypal[os1]?>">
<input type="hidden" name="on2" value="<?=$paypal[on2]?>">
<input type="hidden" name="os2" value="<?=$paypal[os2]?>">

<!-- Shipping and Misc Information --> 
<input type="hidden" name="shipping" value="<?=$paypal[shipping_amount]?>">
<input type="hidden" name="shipping2" value="<?=$paypal[shipping_amount_per_item]?>">
<input type="hidden" name="handling" value="<?=$paypal[handling_amount]?>">
<input type="hidden" name="tax" value="<?=$paypal[tax]?>">
<input type="hidden" name="custom" value="<?=$paypal[custom_field]?>">
<input type="hidden" name="invoice" value="<?=$paypal[invoice]?>">

<!-- Customer Information --> 
<input type="hidden" name="first_name" value="<?=$paypal[firstname]?>"> 
<input type="hidden" name="last_name" value="<?=$paypal[lastname]?>"> 
<input type="hidden" name="address1" value="<?=$paypal[address1]?>"> 
<input type="hidden" name="address2" value="<?=$paypal[address2]?>"> 
<input type="hidden" name="city" value="<?=$paypal[city]?>"> 
<input type="hidden" name="state" value="<?=$paypal[state]?>"> 
<input type="hidden" name="zip" value="<?=$paypal[zip]?>"> 
<input type="hidden" name="email" value="<?=$paypal[email]?>"> 
<input type="hidden" name="night_phone_a" value="<?=$paypal[phone_1]?>"> 
<input type="hidden" name="night_phone_b" value="<?=$paypal[phone_2]?>"> 
<input type="hidden" name="night_phone_c" value="<?=$paypal[phone_3]?>"> 

<?php } ?>