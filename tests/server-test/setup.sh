#!/bin/bash

echo "Attempting to install ansible, java and docker"

#Check if using apt-get
if [ -n "$(command -v apt-get)"  ];
then
    sudo apt-get -y -qq install ansible default-jdk
    sudo apt-get -y -qq install docker-ce
    if [ $? -ne 0 ];
    then
        echo "Please set up the repository for docker-ce.  See https://docs.docker.com/engine/installation/linux/docker-ce/$(. /etc/os-release; echo "$ID")/"
        exit 1
    fi
    sudo systemctl enable docker
    sudo systemctl start docker

    # Add user to docker group so that they can run docker commands
    if [ `groups $USER | grep docker | wc -l` -ne 1 ];
    then
        sudo usermod -a -G docker $USER
        echo "You need to restart your system in order to pick up permission changes"
        exit 1
    fi
fi

#Check if using yum
if [ -n "$(command -v yum)" ]; 
then
    sudo yum -y -q install epel-release
    sudo yum -y -q install ansible java-1.8.0-openjdk docker
    sudo systemctl enable docker
    sudo systemctl start docker
    
    # Add user to dockerroot group so that they can run docker commands
    if [ `groups $USER | grep dockerroot | wc -l` -ne 1 ];
    then
        sudo usermod -a -G dockerroot $USER
        echo "You need to restart your system in order to pick up permission changes"
        exit 1
    fi
fi

# Check if docker-compose is installed.  If it isn't, direct user to instructions
if [ -z "$(command -v docker-compose)" ]; 
then
    echo "Please install docker-compose.  See https://docs.docker.com/compose/install/"
    exit 1
fi


echo "Setup complete."