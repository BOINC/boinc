// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

import com.android.build.gradle.tasks.MergeSourceSetFolders
import org.ajoberstar.grgit.Grgit
import org.gradle.testing.jacoco.plugins.JacocoTaskExtension
import org.gradle.testing.jacoco.tasks.JacocoReport
import org.jetbrains.kotlin.gradle.dsl.JvmTarget
import org.w3c.dom.Element
import java.io.File
import javax.xml.parsers.DocumentBuilderFactory

plugins {
    id("org.ajoberstar.grgit")
    id("jacoco")
    id("com.android.application")
    id("com.android.legacy-kapt")
}

// Use commit date as version code (valid up to 07/18/2036)
fun buildVersionCode(): Int {
    val repo = Grgit.open(mapOf("currentDir" to project.rootDir))
    val versionCode = repo.head().dateTime.toEpochSecond()
    println("versionCode: $versionCode")
    return versionCode.toInt()
}

// Derive version name from release tag and add commit SHA1
fun buildVersionName(): String {
    var version = "8.3.0 : DEVELOPMENT"
    var isDev = true
    var offset = "-1"
    var tag = ""

    val repo = Grgit.open(mapOf("currentDir" to project.rootDir))
    val head = repo.head()
    val tagWithOffset = repo.describe(mapOf("longDescr" to true, "tags" to true))
    val match = Regex("(?<tag>.+)-(?<offset>\\d+)-g(?<hash>[0-9a-f]+)").matchEntire(tagWithOffset ?: "")

    if (match != null) {
        offset = match.groups["offset"]!!.value
        tag = match.groups["tag"]!!.value
    }
    if (offset == "0") {
        val releaseMatch = Regex("client_release/\\d+\\.\\d+/(?<major>\\d+)\\.(?<minor>\\d+)\\.(?<revision>\\d+)")
            .matchEntire(tag)

        // Sanity checks for tag format
        if (releaseMatch != null) {
            val major = releaseMatch.groups["major"]!!.value
            val minor = releaseMatch.groups["minor"]!!.value
            val revision = releaseMatch.groups["revision"]!!.value
            version = "$major.$minor.$revision"
            // Sanity check for tag name
            check(tag == repo.describe(mapOf("tags" to true))) { "Different tag names detected" }
            check(repo.status().isClean) { "Dirty working tree detected! Preventing release build!" }
            isDev = false
        }
    }

    val commit = head.id.take(10)
    val versionName = "$version ($commit)"
    println("versionName: $versionName")
    if (isDev) {
        println("Warning! Non-release tag or offset found: $tag (offset: $offset)")
        println("Flagging as DEVELOPMENT build...")
    }
    return versionName
}

// Parses <string name="...">value</string> elements that are direct children
// of the document root, matching what groovy.xml.XmlParser returned before.
fun parseStringResources(xmlFile: File): Map<String, String> {
    val root = DocumentBuilderFactory.newInstance().newDocumentBuilder().parse(xmlFile).documentElement
    val strings = linkedMapOf<String, String>()
    var node = root.firstChild
    while (node != null) {
        if (node is Element && node.tagName == "string") {
            strings[node.getAttribute("name")] = node.textContent
        }
        node = node.nextSibling
    }
    return strings
}

fun checkAssetsFiles() {
    // The app only uses the default source set, so the res and asset locations are fixed.
    val config = parseStringResources(File(projectDir, "src/main/res/values/configuration.xml"))
    val clientName = config.getValue("client_name")
    val assets = "src/main/assets/"
    listOf("arm64-v8a", "armeabi", "armeabi-v7a", "x86", "x86_64").forEach { abi ->
        check(file("$assets$abi/$clientName").exists()) { "Missing client binary $assets$abi/$clientName" }
    }
    listOf("client_cabundle", "all_projects_list", "nomedia", "client_config").forEach { key ->
        val fileName = config.getValue(key)
        check(file(assets + fileName).exists()) { "Missing asset file $assets$fileName" }
    }
}

tasks.named("preBuild") {
    doFirst {
        checkAssetsFiles()
    }
}

android {
    compileSdk = 34
    namespace = "edu.berkeley.boinc"

    buildFeatures {
        viewBinding = true
        buildConfig = true
        aidl = true
    }

    defaultConfig {
        applicationId = "edu.berkeley.boinc"
        minSdk = 16
        targetSdk = 28
        versionCode = buildVersionCode()
        versionName = buildVersionName()

        // Required when setting minSdkVersion to 20 or lower
        multiDexEnabled = true
        vectorDrawables.useSupportLibrary = true
    }

    buildTypes {
        getByName("release") {
            isMinifyEnabled = true
            isShrinkResources = true
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.txt")
        }

        getByName("debug") {
            isMinifyEnabled = false
            isDebuggable = true
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules-debug.txt")
            enableUnitTestCoverage = true
            enableAndroidTestCoverage = true
        }

        create("armv6_release") {
            initWith(getByName("release"))
        }

        create("armv6_debug") {
            initWith(getByName("debug"))
            isMinifyEnabled = true
            enableUnitTestCoverage = true
            enableAndroidTestCoverage = true
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_21
        targetCompatibility = JavaVersion.VERSION_21

        // Flag to enable support for the new language APIs
        isCoreLibraryDesugaringEnabled = true
    }

    testOptions {
        unitTests {
            isIncludeAndroidResources = true
            all { test ->
                test.useJUnitPlatform()
                test.extensions.configure<JacocoTaskExtension> {
                    isIncludeNoLocationClasses = true
                    excludes = listOf("jdk.internal.*")
                }
                test.jvmArgs = listOf("--add-opens=java.base/java.lang=ALL-UNNAMED", "--add-opens=java.base/java.util=ALL-UNNAMED")
            }
        }
    }
}

tasks.withType<JavaCompile>().configureEach {
    options.compilerArgs.add("-Xlint:unchecked")
    options.isDeprecation = true
}

tasks.withType<Test>().configureEach {
    testLogging {
        events("passed", "skipped", "failed")
    }
}

// Strip non-ARMv6 client binaries from the merged assets of the armv6 variants.
// AGP 9 removed the legacy applicationVariants API, so match the merge tasks by name.
tasks.withType<MergeSourceSetFolders>().configureEach {
    if (name.matches(Regex("mergeArmv6_(release|debug)Assets"))) {
        val mergedAssetsDir = outputDir
        doLast {
            delete(fileTree(mergedAssetsDir) {
                include("**/arm64-v8a/*", "**/armeabi-v7a/*", "**/x86/*", "**/x86_64/*")
            })
        }
    }
}

// Guard for the ARMv6 asset stripping above: fail the armv6 assemble tasks if
// foreign ABI client binaries survive in the merged assets. Also fails loudly
// (unknown task) if AGP ever renames the merge tasks the stripping matches by name.
listOf("Armv6_release", "Armv6_debug").forEach { variantName ->
    val verifyStripped = tasks.register("verify${variantName}AssetsStripped") {
        dependsOn("merge${variantName}Assets")
        doLast {
            val mergeTask = tasks.getByName("merge${variantName}Assets") as MergeSourceSetFolders
            val mergedAssetsDir = mergeTask.outputDir.get().asFile
            val foreignAbiDirs = listOf("arm64-v8a", "armeabi-v7a", "x86", "x86_64").filter { abi ->
                File(mergedAssetsDir, abi).listFiles()?.isNotEmpty() == true
            }
            if (foreignAbiDirs.isNotEmpty()) {
                throw GradleException("ARMv6 merged assets still contain client binaries for: $foreignAbiDirs")
            }
        }
    }
    tasks.matching { it.name == "assemble$variantName" }.configureEach {
        dependsOn(verifyStripped)
    }
}

// The wiring above matches assemble tasks by name; if AGP ever renames them the
// guards would silently stop running. Fail configuration instead.
gradle.projectsEvaluated {
    listOf("Armv6_release", "Armv6_debug").forEach { variantName ->
        if (!tasks.names.contains("assemble$variantName")) {
            throw GradleException("ARMv6 asset-stripping guard is no longer wired: task assemble$variantName not found")
        }
    }
}

kotlin {
    compilerOptions {
        jvmTarget = JvmTarget.JVM_21
    }
}

dependencies {
    coreLibraryDesugaring(Android.tools.desugarJdkLibs)

    // androidx dependencies
    implementation(AndroidX.multidex)
    implementation(AndroidX.appCompat)
    implementation(AndroidX.core.ktx)
    implementation(AndroidX.fragment.ktx)
    implementation(AndroidX.preference.ktx)
    implementation(AndroidX.recyclerView)
    implementation(AndroidX.swipeRefreshLayout)
    implementation(AndroidX.viewPager2)
    implementation("androidx.biometric:biometric:1.1.0")

    implementation("javax.annotation:javax.annotation-api:_")
    implementation("com.github.bumptech.glide:glide:_")
    implementation(Google.android.material)
    implementation("com.google.guava:guava:_")
    implementation(Square.okio)
    implementation("org.apache.commons:commons-lang3:_")
    implementation(Kotlin.stdlib.jdk8)

    // lifecycle dependencies
    implementation(AndroidX.lifecycle.runtime.ktx)
    implementation(AndroidX.lifecycle.service)

    // coroutine dependencies
    implementation(KotlinX.coroutines.core)
    implementation(KotlinX.coroutines.android)

    // dagger dependencies
    implementation(Google.dagger)
    annotationProcessor(Google.dagger.compiler)
    kapt(Google.dagger.compiler)

    // Testing dependencies
    testImplementation(AndroidX.test.core)
    testImplementation("com.google.guava:guava-testlib:_")
    testImplementation(Testing.junit4)
    testImplementation(Testing.robolectric)
    testImplementation(Testing.mockK)
    debugImplementation(AndroidX.fragment.testing)

    // powermock dependencies
    testImplementation("org.powermock:powermock-module-junit4:_")
    testImplementation("org.powermock:powermock-api-mockito2:_")

    // junit dependencies
    testImplementation(Testing.junit.jupiter.api)
    testImplementation(Testing.junit.jupiter.params)
    testRuntimeOnly(Testing.junit.jupiter.engine)
    testRuntimeOnly("org.junit.vintage:junit-vintage-engine:_")
    // Gradle 9 no longer provides the JUnit Platform launcher at runtime
    testRuntimeOnly("org.junit.platform:junit-platform-launcher:_")
}

repositories {
    mavenCentral()
}

// from https://about.codecov.io/blog/code-coverage-for-android-development-using-kotlin-jacoco-github-actions-and-codecov/
tasks.register<JacocoReport>("jacocoTestReport") {
    dependsOn("testDebugUnitTest")

    reports {
        csv.required.set(true)
        xml.required.set(true)
        html.required.set(true)
    }

    val fileFilter = listOf("**/R.class", "**/R\$*.class", "**/BuildConfig.*", "**/Manifest*.*", "**/META-INF*.*", "**/*Test*.*", "android/**/*.*")
    val debugTree = fileTree(layout.buildDirectory.dir("intermediates/javac/debug/classes")) { exclude(fileFilter) } +
            fileTree(layout.buildDirectory.dir("tmp/kotlin-classes/debug")) { exclude(fileFilter) }
    val mainSrc = "$projectDir/src/main/java"

    sourceDirectories.setFrom(files(mainSrc))
    classDirectories.setFrom(files(debugTree))
    executionData.setFrom(fileTree(layout.buildDirectory) {
        include(
            "outputs/unit_test_code_coverage/debugUnitTest/testDebugUnitTest.exec",
            "outputs/code-coverage/debugAndroidTest/connected/coverage.ec"
        )
    })
}
