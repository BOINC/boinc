package tests.general;

import static io.restassured.RestAssured.given;
import static org.hamcrest.Matchers.equalTo;

import org.junit.jupiter.api.Test;

import tests.TestBase;

public class ProjectConfigurationTest extends TestBase {
    
    public static final String TEST_PROJECT_NAME = "boincserver";
    public static final Integer TEST_PROJECT_NUMBER_OF_PLATFORMS = 3;

    @Test
    public void getProjectConfiguration() {
        given()
        .when()
            .get("/get_project_config.php")
        .then()
            .statusCode(200)
            .body("project_config.name", equalTo(TEST_PROJECT_NAME),
                  "project_config.platforms.platform.size()", equalTo(TEST_PROJECT_NUMBER_OF_PLATFORMS),
                  "project_config.master_url", equalTo(projectUrl),
                  "project_config.web_rpc_url_base", equalTo(projectUrl)
                 );
    }

}
