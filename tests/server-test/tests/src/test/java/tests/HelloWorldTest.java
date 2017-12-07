package tests;

import static io.restassured.RestAssured.given;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import io.restassured.RestAssured;

//most basic test which can be used make sure gradle and java environment is correct
public class HelloWorldTest {
    
    @BeforeEach
    public void setup() {
        RestAssured.port = 443;
        RestAssured.basePath = "/";
        RestAssured.baseURI = "www.google.com";

    }
    
    @Test
    public void makeSureThatGoogleIsUp() {
        given()
        .when()
            .get("https://www.google.com")
        .then()
            .statusCode(200);
    }

}
