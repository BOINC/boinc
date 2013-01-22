#/bin/sh
#script to compile BOINC for Android

#++++++++++++++++++++++++CONFIGURATION++++++++++++++++++++++++++++

export BOINC=".." #BOINC source code
export OPENSSL_DIR=$BOINC/../boinc_depends_android_eclipse/openssl
export CURL_DIR=$BOINC/../boinc_depends_android_eclipse/curl

#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

echo "===================copy binary to assets direcotry of APK at $TARGETAPK========================="

mkdir "BOINC/assets"
cp "$BOINC/stage/usr/local/bin/boinc" "BOINC/assets/boinc"
cp "$CURL_DIR/ca-bundle.crt" "BOINC/assets/ca-bundle.crt"

echo "============================script done=========================="
