<?php
declare(strict_types=1);

use PHPUnit\Framework\TestCase;

final class CreateAccountTest extends TestCase
{
    const MISSING_PARAM_ERROR = "missing or bad parameter";
    const MISSING_PASSWORD_ERROR = "missing or bad parameter: passwd_hash";
    const MISSING_EMAIL_ERROR = "missing or bad parameter: email_addr";
    const PASSWORD = "123456"; # https://en.wikipedia.org/wiki/List_of_the_most_common_passwords
    const USERNAME_BASE = "random";
    const EMAIL_DOMAIN = "@example.com";
    var $user;
    var $email;
    var $hash;

    public function setUp(): void
    {
       $base_url = sprintf("%s://%s/%s/", protocol, host, base );
       $this->http = new GuzzleHttp\Client(['base_uri' => $base_url]);
       $this->user = self::USERNAME_BASE . rand();
       $this->email = $this->user . self::EMAIL_DOMAIN;
       $this->hash = md5(self::PASSWORD . $this->email);
    }

    public function testCreateAccountNoParams()
    {
        $response = $this->http->request('GET', 'create_account.php');

	$this->assertEquals(200, $response->getStatusCode());

	$xml = (string) $response->getBody();
    	$xml = simplexml_load_string($xml);
        $this->assertStringStartsWith(self::MISSING_PARAM_ERROR, (string) $xml->error_msg);
    }

    public function testCreateAccountEmailAndUserNameNoPassword()
    {
	$query = [
	         'user_name' => $this->user,
                 'email_addr' => $this->email,
	        ];
	$response = $this->http->request('GET', 'create_account.php',  ['query' => $query]);

	$this->assertEquals(200, $response->getStatusCode());

	$xml = (string) $response->getBody();
    	$xml = simplexml_load_string($xml);

        $this->assertStringStartsWith(self::MISSING_PASSWORD_ERROR, (string) $xml->error_msg);
    }

    public function testCreateAccountPasswordAndUserNameNoEmail()
    {
	$query = [
	         'user_name' => $this->user,
		 'passwd_hash' => $this->hash,
	        ];
        $response = $this->http->request('GET', 'create_account.php',  ['query' => $query]);

	$this->assertEquals(200, $response->getStatusCode());

	$xml = (string) $response->getBody();
    	$xml = simplexml_load_string($xml);
	$this->assertStringStartsWith(self::MISSING_EMAIL_ERROR, (string) $xml->error_msg);
    }

    public function testCreateAccount()
    {

	$query = [
	         'user_name' => $this->user,
	         'email_addr' => $this->email,
                 'passwd_hash' => $this->hash,
	        ];
	$response = $this->http->request('GET', 'create_account.php',  ['query' => $query]);

	$this->assertEquals(200, $response->getStatusCode());

	$xml = (string) $response->getBody();
    	$xml = simplexml_load_string($xml);
        $authenticator = (string) $xml->authenticator;
	$this->assertCount( 1, $xml->authenticator);
    }

    public function testCreateAndLookupAccount()
    {
	$query = [
                 'email_addr' => $this->email,
		 ];
	$response = $this->http->request('GET', 'lookup_account.php',  ['query' => $query]);

	$this->assertEquals(200, $response->getStatusCode());

	$xml = (string) $response->getBody();
    	$xml = simplexml_load_string($xml);
        $this->assertCount( 0, $xml->success);
    }

    public function tearDown(): void
    {
        $this->http = null;
    }

}
