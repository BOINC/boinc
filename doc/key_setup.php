<?php
require_once("docutil.php");
page_head("Creating encryption keys");
echo "
<h2>Creating encryption keys</h2>


The following commands generate the file upload and code signing key pairs.
BOINC_KEY_DIR is the directory where the keys will be stored.
The code signing private key should be stored only on
a highly secure (e.g., a disconnected, physically secure) host.
<pre>
crypt_prog -genkey 1024 BOINC_KEY_DIR/upload_private BOINC_KEY_DIR/upload_public
crypt_prog -genkey 1024 BOINC_KEY_DIR/code_sign_private BOINC_KEY_DIR/code_sign_public
</pre>
Or, in the test/ directory, run
<pre>
gen_keys.php
</pre>

<p>
The program <b>lib/crypt_prog</b> can be used for several purposes: 
<br>
<dl>
<dt>crypt_prog -genkey n private_keyfile public_keyfile
<dd>
Create a key pair with n bits (always use 1024).
Write the keys in encoded ASCII form to the indicated files. 
<dt>crypt_prog -sign file private_keyfile
<dd>
Create a digital signature for the given file. Write it in encoded
ASCII to stdout. 
<dt>crypt_prog -verify file signature_file public_keyfile
<dd>
Verify a signature for the given file. 
<dt>crypt_prog -test_crypt private_keyfile public_keyfile
<dd>
Perform an internal test, checking that encryption followed by
decryption works. 
</dl>
";
page_tail();
?>
