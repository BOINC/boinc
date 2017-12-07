package tests.users;

import static io.restassured.RestAssured.given;
import static org.hamcrest.Matchers.startsWith;
import static org.hamcrest.Matchers.equalTo;

import org.junit.jupiter.api.Test;

import io.restassured.response.Response;
import tests.TestBase;
import util.GenerateRandomUser;
import util.User;

/*
 * These tests test the BOINC Web RPC's to create and get account
 */
public class CreateAccountTest extends TestBase {
    
    private static String FAIL_PARAMETER_EMAIL = "test-fail@test1.com";
    private static String FAIL_PARAMETER_USERNAME = "test-fail";
    
    private static String MISSING_PARAM_ERROR = "missing or bad parameter";
    private static String MISSING_PASSWORD_ERROR = "missing or bad parameter: passwd_hash";
    
    @Test
    public void testCreateAccountNoParams() {
        given()
        .when()
            .get("/create_account.php")
        .then()
            .statusCode(200)
            .body("error.error_msg", startsWith(MISSING_PARAM_ERROR)
                 );
       
    }
    
    @Test
    public void testCreateAccountEmailAndUserNameNoPassword() {
        given()
            .param("email_addr", FAIL_PARAMETER_EMAIL)
            .param("user_name", FAIL_PARAMETER_USERNAME)
        .when()
            .get("/create_account.php")
        .then()
            .statusCode(200)
            .body("error.error_msg", startsWith(MISSING_PASSWORD_ERROR)
                 );
    }
    
    @Test
    public void testCreateAccountPasswordAndUserNameNoEmail() {
        given()
            .param("email_addr", FAIL_PARAMETER_EMAIL)
            .param("user_name", FAIL_PARAMETER_USERNAME)
        .when()
            .get("/create_account.php")
        .then()
            .statusCode(200)
            .body("error.error_msg", startsWith(MISSING_PASSWORD_ERROR)
                 );
    }
    
    @Test
    public void testCreateAccount() {
        User user = GenerateRandomUser.createUser();
        createRandomAccount(user);
    }
    
    private Response createRandomAccount(User user) {
        Response response = given()
                .param("email_addr", user.getEmailAddress())
                .param("user_name", user.getUserName())
                .param("passwd_hash", user.getPasswordHash())
            .when()
                .get("/create_account.php");

        response.then()
            .statusCode(200)
            .body("account_out.authenticator.size()", equalTo(1) );

        return response;
    }
    
    @Test
    public void testCreateAndLookupAccount() {
        User user = GenerateRandomUser.createUser();
        Response response = createRandomAccount(user);
        
        String authenticator = response.xmlPath().getString("account_out.authenticator");
        given()
            .param("email_addr", user.getEmailAddress())
            .param("user_name", user.getUserName())
            .param("passwd_hash", user.getPasswordHash())
        .when()
            .get("/lookup_account.php")
        .then()
            .body("account_out.authenticator", equalTo(authenticator));
               
        
    }
    
}
