#!/bin/bash

echo "Attempting to install ansible, java and docker"

#Check if using apt-get
if [ -n "$(command -v apt-get)"  ];
then
    sudo apt-get -y -qq install ansible
    sudo apt-get -y -qq install docker-ce
    if [ $? -ne 0 ];
    then
        echo "Please set up the repository for docker-ce.  See https://docs.docker.com/engine/installation/linux/docker-ce/$(. /etc/os-release; echo "$ID")/"
        exit 1
    fi
fi

#Check if using yum
if [ -n "$(command -v yum)" ];
then
    sudo yum -y -q install epel-release
    sudo yum -y -q install ansible docker composer
fi

# Add user to docker group so that they can run docker commands
if [ `groups $USER | grep docker | wc -l` -ne 1 ];
then
    if ! grep -q docker: /etc/group
    then
         sudo groupadd docker
    fi
    sudo usermod -a -G docker $USER
    echo "You have been added to the docker group.  In order to continue you may need to log out and back in again or restart your system in order to pick up the group change"
    exit 1
fi

# Check if docker compose is installed.  If it isn't, direct user to instructions
if [ -z "$(command -v docker compose)" ];
then
    echo "Please install docker compose.  See https://docs.docker.com/compose/install/"
    exit 1
fi

# Check if composer is installed.  If it isn't, direct user to instructions
if [ -z "$(command -v composer)" ];
then
    echo "Please download and install composer.  See https://getcomposer.org/download/"
    exit 1
fi

sudo systemctl enable docker
sudo systemctl restart docker

cd tests
composer require phpunit/phpunit
composer require guzzlehttp/guzzle
composer update
cd ..

echo "Setup complete."
