package util;

import java.util.Random;

import org.apache.commons.codec.digest.DigestUtils;

public class GenerateRandomUser {
    
    private static final String TEST_PASSWORD = "testtest";
    private static final String TEST_USERNAME_BASE = "test";
    private static final String TEST_EMAIL_DOMAIN = "@example.com";
    
    public static User createUser() {
        User user = new User();
        user.setUserName(getUserName());
        user.setEmailAddress(getEmailAddr(user.getUserName()));
        user.setPasswordHash(getPasswordHash(user.getEmailAddress()));
        return user;
    }

    //generates a random email address
    private static String getUserName() {
        int randomNum = new Random().nextInt();
        return TEST_USERNAME_BASE + randomNum;
    }
    
    private static String getEmailAddr(String userName) {
        return userName + TEST_EMAIL_DOMAIN;
    }
    
    //generates password hahsed with email address
    private static String getPasswordHash(String emailAddress) {
        return DigestUtils.md5Hex(TEST_PASSWORD + emailAddress.toLowerCase()).toLowerCase();
    }
    
}
