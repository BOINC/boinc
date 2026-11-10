include(":app")

buildscript {
    val refreshVersionsVersion = "0.60.6"
    repositories {
        maven {
            url = uri("https://plugins.gradle.org/m2/")
            name = "Gradle Plugin Portal"
        }
    }
    dependencies {
        classpath("de.fayard.refreshVersions:refreshVersions:$refreshVersionsVersion")
    }
}

apply(plugin = "de.fayard.refreshVersions")
