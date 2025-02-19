# boinc-server-test
This project contains code that will build BOINC in docker containers based on https://github.com/marius311/boinc-server-docker and then execute a series of tests.

# Initial setup
Run the setup.sh command to install ansible and docker.  Note that you might need to take actions that it recommends when you run the script.  If you do, you should re-run the script to confirm correct installation.  You will know that you are done when you see the text **Setup complete.**

# Build and start a BOINC website
The following commands are run from the **_manage_** subdirectory.

## Build docker containers from BOINC master
The default usage is to build the docker containers using the code from the master branch of https://github.com/BOINC/boinc.  This can be done using the following command:

```
ansible-playbook -i hosts build.yml
```

## Build docker containers from a different branch and repository
If you need to build from a different branch and/or repository, you can specify the repository and branch to use.  For example, to build from a hypothetical branch in my fork of the BOINC repository you can do use:

```
ansible-playbook -i hosts build.yml --extra-vars "boinc_repository=https://github.com/TheAspens/boinc boinc_branch=split_boinc_website"
```

## Build docker containers from a local version of boinc
If you need to build from a local branch and/or repository, you can specify the directory to build from to use.  For example, to build from the code in the boinc directory in my home directory you would do:

```
ansible-playbook -i hosts build.yml --extra-vars "boinc_dir=/home/knreed/boinc"
```

## Start docker containers (Start BOINC project)
Once you have built the version of BOINC you want, you can start the project by starting the containers:
```
ansible-playbook -i hosts start.yml
```

You can now access the new website via http://127.0.0.1/boincserver.  Please see https://github.com/marius311/boinc-server-docker for more options of what you can do with the running docker containers.

## Stop docker containers and remove volumes
If you want to stop the docker containers and remove their associated storage (so that next time you start them, it will be like a brand new website) then run the following command:
```
ansible-playbook -i hosts stop_and_remove.yml
```

## Handy Commands for interacting with your BOINC website
Access bash prompt in apache container:
```
docker exec -it boinc-server-docker_apache_1 bash
```

# Automated Tests
The tests are written to be automation oriented integration tests.  At the moment the tests are simply testing a few of the [BOINC Web RPC's](https://boinc.berkeley.edu/trac/wiki/WebRpc).  They are executed using [PHPUnit](https://phpunit.de/).

Tests can easily be added to this framework to test the scheduler, file uploads and more using what is already set up.


If we want to extend this to test the web interface, we get leverage a platform like Selenium WebDriver and run it under PHPUnit like we are doing with other tests.

## Run Tests
Tests are run out of the **_tests_** subdirectory.  They require the BOINC project running (see above).  These test are run with the following command:
```
composer test
```
