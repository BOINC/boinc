#!/bin/bash

echo "Attempting to install ansible and docker"

#Check if using apt-get
if [ -n "$(command -v apt-get)"  ];
then
   sudo apt-get -y install ansible
   sudo apt-get -y install docker-engine
    if [ $? -ne 0 ];
    then
        echo "Please set up the repository for docker-ce.  See https://docs.docker.com/engine/installation/linux/docker-ce/$(. /etc/os-release; echo "$ID")/"
    fi
    sudo usermod -a -G docker $USER
fi

#Check if using yum
if [ -n "$(command -v yum)" ]; 
then
    sudo yum -y install ansible docker
    if [ $? -ne 0 ];
    then
        echo "Please enable the EPEL repository in order to install ansible.  See https://fedoraproject.org/wiki/EPEL"
    fi
    sudo usermod -a -G docker $USER
fi
