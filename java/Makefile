# make file for java wrapper sample
# don't forget to adapt the directories

BOINC_DIR = ../../boinc
BOINC_API_DIR = $(BOINC_DIR)/api
BOINC_LIB_DIR = $(BOINC_DIR)/lib

# location of jni.h
JNI_INCLUDE_DIR = /usr/lib/jvm/java-1.5.0-sun-1.5.0_update10/include/
# location of platform specific include files
JNI_INCLUDE_PLATFORM_DIR = /usr/lib/jvm/java-1.5.0-sun-1.5.0_update10/include/linux
# location of libjvm.so
JVM_LIB_DIR = /usr/lib/jvm/java-1.5.0-sun-1.5.0_update10/jre/lib/i386/client

CXXFLAGS = -g \
    -I$(BOINC_DIR) \
    -I$(BOINC_LIB_DIR) \
    -I$(BOINC_API_DIR) \
    -I$(JNI_INCLUDE_DIR) \
    -I$(JNI_INCLUDE_PLATFORM_DIR) \
    -L$(BOINC_API_DIR) \
    -L$(BOINC_LIB_DIR) \
    -L$(JVM_LIB_DIR)

PROGS = uppercase_invoke libboincjava.so

all: java $(PROGS)

clean:
	-rm $(PROGS)
	-rm UpperCase.class BOINCJavaWrapper.class BOINCJavaWrapper.h
	-rm *.o

java: BOINCJavaWrapper.class UpperCase.class

BOINCJavaWrapper.class: BOINCJavaWrapper.java
	javac BOINCJavaWrapper.java
	javah -jni BOINCJavaWrapper

UpperCase.class: UpperCase.java
	javac UpperCase.java

uppercase_invoke: uppercase_invoke.o $(BOINC_API_DIR)/libboinc_api.a $(BOINC_LIB_DIR)/libboinc.a
	$(CXX) $(CXXFLAGS) -o uppercase_invoke uppercase_invoke.o -ljvm -lpthread

libboincjava.so: BOINCJavaWrapper.o $(BOINC_API_DIR)/libboinc_api.a $(BOINC_LIB_DIR)/libboinc.a BOINCJavaWrapper.class
	$(CXX) $(CXXFLAGS) -o libboincjava.so -shared -Wl,-soname,libboincjava.so BOINCJavaWrapper.C -lboinc_api -lboinc -lpthread
