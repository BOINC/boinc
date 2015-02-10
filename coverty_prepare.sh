#!/bin/bash

sudo apt-get install python libssl-dev python-mysqldb libmysqlclient-dev libfcgi-dev libcurl4-openssl-dev libxss-dev libnotify-dev libxcb-util0-dev -qq
echo "package install complete";
./_autosetup
echo "autosetup complete";
./configure --disable-manager
echo "configure complete";
