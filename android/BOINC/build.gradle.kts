// Top-level build file where you can add configuration options common to all sub-projects/modules.
buildscript {
    repositories {
        maven {
            url = uri("https://maven.google.com/")
            name = "Google"
        }
        maven {
            url = uri("https://repo1.maven.org/maven2/")
            name = "Central Repository"
        }
        google()
    }
    dependencies {
        classpath(Android.tools.build.gradlePlugin)
        // Provides the com.android.legacy-kapt plugin (kapt support for AGP's built-in Kotlin)
        classpath("com.android.tools.build:gradle-kotlin:_")
        classpath("org.jacoco:org.jacoco.core:_")
    }
}

allprojects {
    repositories {
        maven {
            url = uri("https://maven.google.com/")
            name = "Google"
        }
    }
}

tasks.register("jacocoTestReportDebug") {
    dependsOn("app:jacocoTestReport")
}
