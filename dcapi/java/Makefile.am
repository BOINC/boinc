AM_CPPFLAGS = $(JAVA_INC)

# We need the following build order:
# 1. Java classes
# 2. dc-client.jar
# 3. jni_proto.h
# 4. libdcapi-java-client.la


#######################################################################
# Rules for building the Java part

# Directory prefix of the classes
CLASSDIR = hu/sztaki/lpds/dc

# Generate debugging information by default
AM_JAVACFLAGS = -g

COMMON_SRCS = \
		$(CLASSDIR)/DCException.java \
		$(CLASSDIR)/FileMode.java \
		$(CLASSDIR)/FileType.java \
		$(CLASSDIR)/RuntimeDCException.java
CLIENT_SRCS = \
		$(CLASSDIR)/client/Event.java \
		$(CLASSDIR)/client/DCClient.java \
		$(CLASSDIR)/client/Version.java

DOCFILES = \
		$(CLASSDIR)/overview.html \
		$(CLASSDIR)/package.html

noinst_JAVA = $(COMMON_SRCS) $(CLIENT_SRCS)

COMMON_CLASSES = $(COMMON_SRCS:.java=.class)
COMMON_CLASSNAMES = $(subst /,.,$(COMMON_CLASSES:.class=))

CLIENT_CLASSES = $(CLIENT_SRCS:.java=.class)
CLIENT_CLASSNAMES = $(subst /,.,$(CLIENT_CLASSES:.class=))

COMMON_JAR = dc-common-$(PACKAGE_VERSION).jar
CLIENT_JAR = dc-client-$(PACKAGE_VERSION).jar
JARFILES = $(COMMON_JAR) $(CLIENT_JAR)

javadir = $(datadir)/dc-api
java_DATA = $(JARFILES)

install-data-hook:
	cd $(DESTDIR)$(javadir) && \
	rm -f dc-common.jar && \
	rm -f dc-client.jar && \
	$(LN_S) $(COMMON_JAR) dc-common.jar && \
	$(LN_S) $(CLIENT_JAR) dc-client.jar

# Override automake defaults
CLASSPATH_ENV =
JAVAROOT = $(top_builddir)/java

# Depending on $(CLASSFILES) does not work. This solution is not documented
# in the automake manual but works with automake-1.7
$(CLIENT_JAR): classnoinst.stamp
	$(JAR) -cf $@ $(CLIENT_CLASSES)

$(COMMON_JAR): classnoinst.stamp
	$(JAR) -cf $@ $(COMMON_CLASSES)

$(CLASSDIR)/client/Version.java: $(CLASSDIR)/client/Version.java.in $(top_builddir)/configure.ac
	cd $(top_builddir) && $(SHELL) ./config.status java/$(CLASSDIR)/client/Version.java


#######################################################################
# Rules for building the JNI part

lib_LTLIBRARIES = \
		libdcapi-java-client.la
noinst_HEADERS = util.h
BUILT_SOURCES = jni_proto.h jni_export.sym

libdcapi_java_client_la_SOURCES = \
		client.c \
		util.c
libdcapi_java_client_la_LDFLAGS = \
		-avoid-version \
		-release $(PACKAGE_VERSION) \
		-export-symbols=jni_export.sym
libdcapi_java_client_la_LIBADD = \
		$(top_builddir)/boinc/libdcapi-client-boinc.la
libdcapi_java_client_la_DEPENDENCIES = jni_export.sym

# jni_proto.h contains both the consumer and producer-side JNI prototypes
jni_proto.h: $(JARFILES)
	$(JAVAH) -classpath $(COMMON_JAR):$(CLIENT_JAR) -o $@ -jni $(COMMON_CLASSNAMES) $(CLIENT_CLASSNAMES)
	-touch $@

# Generate the list of to-be-exported symbols from jni_proto.h
# This is a little fragile but works
jni_export.sym: jni_proto.h
	grep '^JNIEXPORT.*JNICALL ' $< | \
		sed -e 's/JNIEXPORT.*JNICALL //' > $@
	echo "JNI_OnLoad" >> $@
	echo "JNI_OnUnload" >> $@

# On Darwin, the JVM wants a .jnilib extension instead of the native .dylib
install-exec-hook:
	case $(host_os) in \
		darwin*) \
			$(LN_S) -f $(DESTDIR)$(libdir)/libdcapi-java-client-$(PACKAGE_VERSION).dylib $(DESTDIR)$(libdir)/libdcapi-java-client-$(PACKAGE_VERSION).jnilib; \
	esac


#######################################################################
# Misc. rules

EXTRA_DIST = \
		$(noinst_JAVA) \
		$(DOCFILES)

MOSTLYCLEANFILES = \
		$(COMMON_CLASSES) \
		$(CLIENT_CLASSES) \
		$(JARFILES) \
		javacore.txt \
		core.*

CLEANFILES = \
		doc.stamp \
		jni_export.sym \
		jni_proto.h


#######################################################################
# Build and install the API documentation

TARGET_DIR = $(docdir)/java

doc.stamp: classnoinst.stamp $(DOCFILES)
	-mkdir $(top_builddir)/java/doc 2>/dev/null
	$(JAVADOC) -quiet -d $(top_builddir)/java/doc \
		-sourcepath $(srcdir) \
		-classpath $(COMMON_JAR):$(CLIENT_JAR) \
		-overview $(srcdir)/$(CLASSDIR)/overview.html \
		hu.sztaki.lpds.dc
	-touch $@

all-local: doc.stamp

clean-local:
	-rm -rf doc

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(TARGET_DIR)/$(CLASSDIR)
	$(mkinstalldirs) $(DESTDIR)$(TARGET_DIR)/$(CLASSDIR)/producer
	(installfiles=`echo $(srcdir)/doc/*.html $(srcdir)/doc/*.css`; \
	if test "$$installfiles" = '$(srcdir)/doc/*.html $(srcdir)/doc/*.css'; \
	then echo '-- Nothing to install' ; \
	else \
	  for i in $$installfiles; do \
	    echo '-- Installing '$$i ; \
	    $(INSTALL_DATA) $$i $(DESTDIR)$(TARGET_DIR); \
	  done; \
	  echo '-- Installing $(srcdir)/doc/package-list' ; \
	  $(INSTALL_DATA) $(srcdir)/doc/package-list $(DESTDIR)$(TARGET_DIR); \
	fi)
	(installfiles=`echo $(srcdir)/doc/$(CLASSDIR)/*.html`; \
	if test "$$installfiles" = '$(srcdir)/doc/$(CLASSDIR)/*.html'; \
	then echo '-- Nothing to install' ; \
	else \
	  for i in $$installfiles; do \
	    echo '-- Installing '$$i ; \
	    $(INSTALL_DATA) $$i $(DESTDIR)$(TARGET_DIR)/$(CLASSDIR); \
	  done; \
	fi)
	(installfiles=`echo $(srcdir)/doc/$(CLASSDIR)/producer/*.html`; \
	if test "$$installfiles" = '$(srcdir)/doc/$(CLASSDIR)/producer/*.html'; \
	then echo '-- Nothing to install' ; \
	else \
	  for i in $$installfiles; do \
	    echo '-- Installing '$$i ; \
	    $(INSTALL_DATA) $$i $(DESTDIR)$(TARGET_DIR)/$(CLASSDIR)/producer; \
	  done; \
	fi)
