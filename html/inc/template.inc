<?php

function doHeader($caption) {
?>

<!DOCTYPE HTML PUBLIC "-//W3C//Dtd HTML 4.01 Transitional//EN">

<html>

<head>
	<title><?php echo $GLOBALS['cfg']['sitename'].' - '.$caption ?></title>
	<meta http-equiv="content-type" content="text/html; charset=<?php echo (!empty($_SESSION['lang']['charset']))?$_SESSION['lang']['charset']:'ISO-8859-1' ?>">
	<link rel="stylesheet" type="text/css" href="<?php echo $GLOBALS['cfg']['root'] ?>/style.css">
</head>

<body>

<table class="frame" border="0" cellpadding="10" cellspacing="0" width="100%">
	<tr>
		<td>
<?php
}

function doFooter() {
?>
<p style="text-align:center"><a href="http://maggie.ssl.berkeley.edu/ap">Return to <?php echo $GLOBALS['cfg']['sitename'] ?> Homepage</a></p>

		</td>
	</tr>
</table>

</body>

</html>

<?php
}

?>
