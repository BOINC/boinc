# boinc-server-test
This project contains code that will build BOINC in docker containers based on https://github.com/marius311/boinc-server-docker and then execute a series of tests.

# Intial setup
Run the setup.sh command to install ansible and docker

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
docker exec -it boincserverdocker_apache_1 bash
```

# Executing tests

## Install gradle
Please follow the instructions at https://gradle.org/install/#manually to install gradle.

## Run Tests
- Start servers (see above)
- Execute tests
```
gradle test
```