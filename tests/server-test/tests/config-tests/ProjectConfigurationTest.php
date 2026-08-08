<?php
declare(strict_types=1);

use PHPUnit\Framework\TestCase;

final class ProjectConfigurationTest extends TestCase
{

    public function setUp(): void
    {
       $base_url = sprintf("%s://%s/%s/", protocol, host, base );
       $this->http = new GuzzleHttp\Client(['base_uri' => $base_url]);
    }

    public function testGet()
    {
	$response = $this->http->request('GET', 'get_project_config.php');

	$this->assertEquals(200, $response->getStatusCode());

	$xml = (string) $response->getBody();
    	$xml = simplexml_load_string($xml);

        $base_url = sprintf("%s://%s/%s/", protocol, host, base );

        $this->assertEquals(base, (string) $xml->name);
//  Disable check of platform counts because without boinc2docker installed, no platforms are installed
//  This can be re-enabled once boinc2docker supports python3
//	$this->assertCount( (int) num_platforms, $xml->platforms->platform);
	$this->assertEquals($base_url, (string) $xml->master_url);
	$this->assertEquals($base_url, (string) $xml->web_rpc_url_base);
    }

    public function tearDown(): void
    {
        $this->http = null;
    }

}
