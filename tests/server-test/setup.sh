#!/bin/bash

echo "Attempting to install ansible"

#Check if using apt-get
if [ -n "$(command -v apt-get)"  ];
then
   sudo apt-get -y install ansible
fi

#Check if using yum
if [ -n "$(command -v yum)" ]; 
then
    sudo yum -y install ansible
    if [ $? -ne 0 ];
    then
        echo "Please enable the EPEL repository in order to install ansible.  See https://fedoraproject.org/wiki/EPEL"
    fi
fi
