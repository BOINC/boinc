<?php
require_once('include.php');
doHeader('New Account');

if (parse_config("<disable_account_creation/>")):
	?>
	<h1>Account Creation is Disabled</h1>
	<p>
	Account creation is disabled for <?php echo $cfg['project'] ?> at the moment.
	Please try again later.
	</p>
	<?php
	doFooter();
	exit;
endif;
?>

<h1>Create an Account with <?php echo $cfg['project'] ?></h1>

<p><b>Read the <a href="info.php">Rules and Policies</a> before creating an account.</b></p>

<p>If you already received an account key, do not submit this form.  <a href="account_created.php">Activate your account</a> instead.</p>

<form action="create_account_form.php" method="post">
	<?php if (!empty($_GET['userid'])): ?><input type="hidden" name="userid" value="<?php echo $_GET['userid'] ?>"><?php endif; ?>
	<table class="content" border="0" cellpadding="5" cellspacing="0">
		<tr>
			<th colspan="2">Create Account</th>
		</tr>
		<tr>
			<td class="item">
				Name
				<br><span class="description">Identifies you on our web site. Use your real name or a nickname.</span>
			</td>
			<td class="col1"><input name="new_name" size="30"></td>
		</tr>
		<tr>
			<td class="item">
				Email Address
				<br><span class="description">Must be a valid address of the form 'name@domain'.</span>
			</td>
			<td class="col1"><input name="new_email_addr" size="50"></td>
		</tr>
		<tr>
			<td class="item">
				Country
				<br><span class="description">Select the country you want to represent, if any.</span>
			</td>
			<td class="col1">
				<select name="country">
					<?php print_country_select(); ?>
				</select>
			</td>
		</tr>
		<tr>
			<td class="item">
				Postal or ZIP Code
				<br><span class="description">Optional.</span>
			</td>
			<td class="col1"><input name="postal_code" size="20"></td>
		</tr>
	</table>
	<p style="text-align:center"><input type="submit" value="Create Account"></p>
</form>

<?php
doFooter();
?>

