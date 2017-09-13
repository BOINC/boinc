# boinc-server-test
This project contains code that will build BOINC in docker containers based on https://github.com/marius311/boinc-server-docker and then execute a series of tests.

# Build docker containers from BOINC master
The default usage is to build the docker containers using the code from the master branch of https://github.com/BOINC/boinc.  This can be done using the following command:

```
ansible-playbook -K -i hosts build.yml
```

Once that command completes, you can start the containers by doing the following:
```
cd /tmp/boinc-server-docker
docker-compose up -d
```

You can now access the new website via http://127.0.0.1/boincserver.  Please see https://github.com/marius311/boinc-server-docker for more options of what you can do with the running docker containers.

# Build docker containers from a different branch and repository
If you need to build from a different branch and/or repository, you can specify the repository and branch to use.  For example, to build from a hypothetical branch in my fork of the BOINC repository you can do use:

```
ansible-playbook -K -i hosts build.yml --extra-vars "boinc_repository=https://github.com/TheAspens/boinc boinc_branch=split_boinc_website
```

# TBD
Now that I a specific build of BOINC can be done, I need to add logic to start the docker containers and execute tests.  
