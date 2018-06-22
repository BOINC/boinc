# Installing on Linux
The recomended way to install the BOINC client on Linux is to use the package management system of your Linux distribution. If the BOINC client does not exist for your distribtion, please [open an issue](https://github.com/BOINC/boinc/issues/new) in github. Note that there are two packages, one for BOINC Client and another for BOINC Manager, which can be installed separately. Only the client is required but it is likely that the manager is wanted as well unless the client is to be managed from a remote host.
## Debian
Open a terminal and enter the following command:
```
sudo apt-get install boinc-client boinc-manager
```
## Ubuntu
Use the software centre to install the BOINC manager or following instructions for Debian.

## Fedora
Open a terminal and enter the following command:
```
sudo dnf install boinc-manager
```
## Redhat/CentOS
To Ensure that the EPEL repository is enabled, open a terminal and enter the following command:
```
wget http://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
yum localinstall http://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
```
Open a terminal and enter the following command:
```
sudo yum install boinc-client boinc-manager
```
## Building from Source
The source code for BOINC can be obtained from the [github repository](https://github.com/BOINC/boinc). This can be done with the following command:
```
git clone https://github.com/BOINC/boinc.git
```
It can the be built with the follwoing commands
```
cd boinc
./_autosetup
./configure \
  --disable-silent-rules \
  --enable-dynamic-client-linkage \
  --disable-server \
  --disable-fcgi \
  --enable-unicode \
  --with-wx-config=/usr/bin/wx-config-3.0 \
  --with-ssl \
  --with-x \
  STRIP=: \
  DOCBOOK2X_MAN=/usr/bin/db2x_docbook2man \
  "CXXFLAGS=$(pkg-config gtk+-x11-3.0 --cflags --libs) -DNDEBUG"
make 
make install 
```
Alternatively a specific version (gitbranch and gittag) can be downloaded with the following command:
```
wget https://github.com/BOINC/boinc/archive/${gitbranch}/boinc-client-${gittag}.tar.gz
```

