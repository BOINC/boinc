<?php

function doHeader($caption) {
?>

<!DOCTYPE HTML PUBLIC "-//W3C//Dtd HTML 4.01 Transitional//EN">

<html>

<head>
	<title><?php echo PROJECT.' - '.$caption ?></title>
	<meta http-equiv="content-type" content="text/html; charset=<?php echo (!empty($_SESSION['lang']['charset']))?$_SESSION['lang']['charset']:'ISO-8859-1' ?>">
	<link rel="stylesheet" type="text/css" href="/ap/style.css">
</head>

<body>

<table class="frame" border="0" cellpadding="0" cellspacing="10" width="100%">
	<tr>
		<td style="background:url('http://setiathome.berkeley.edu/images/outrspc3.gif')">
			<img src="/ap/image/banner.gif" />
		</td>
	</tr>
	<tr>
		<td>
<?php
}

function doFooter() {
?>
		</td>
	</tr>
	<tr>
		<td style="text-align:center">
			<hr style="width=50%">
			<p><a href="/ap/index.php">Return to <?php echo PROJECT ?> homepage</a></p>
			<p>Copyright (c) <?php echo date('Y') ?> <?php echo PROJECT ?>.  All rights reserved.</p>
		</td>
	</tr>
</table>

</body>

</html>

<?php
}

?>
