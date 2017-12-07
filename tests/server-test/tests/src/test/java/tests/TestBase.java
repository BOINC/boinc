package tests;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;
import io.restassured.RestAssured;

import org.junit.jupiter.api.BeforeEach;

import static org.junit.jupiter.api.Assertions.assertNotNull;

public class TestBase {
    
    protected String projectUrl;
    
    @BeforeEach
    public void setup() throws FileNotFoundException, IOException {
        Properties props = new Properties();
        InputStream propretiesStream = TestBase.class.getResourceAsStream("/Environment.properties");
        assertNotNull(propretiesStream, "Unable to locate resource Environment.properties file");
        props.load(propretiesStream);
        
        RestAssured.port = Integer.valueOf(props.getProperty("server.port"));
        RestAssured.basePath = props.getProperty("server.base");
        RestAssured.baseURI = props.getProperty("server.host");
        
        projectUrl = props.getProperty("server.host") + props.getProperty("server.base") + "/";
    }

}
