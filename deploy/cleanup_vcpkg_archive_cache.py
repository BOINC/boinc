import os
import sys
import zipfile
import s3

def get_files(dir):
    file_names = []
    for path, _, files in os.walk(dir):
        for name in files:
            file_name = os.path.join(path, name)
            file_names.append(file_name)
    return file_names
        
def read_control(control):
    package = ''
    version = '0'
    port_version = '0'
    architecture = ''
    lines = control.split('\n')
    for line in lines:
        if (line != ''):
            pair = line.split(': ')
            if (pair[0] == 'Package'):
                package = pair[1]
            elif (pair[0] == 'Version'):
                version = pair[1]
            elif (pair[0] == 'Port-Version'):
                port_version = pair[1]
            elif (pair[0] == 'Architecture'):
                architecture = pair[1]                   
    return package, version + '-' + port_version, architecture

def get_packages(archives):
    packages = {}
    for archive in archives:
        zip_file = zipfile.ZipFile(archive, 'r')
        control = zip_file.read('CONTROL')
        package, version, architecture = read_control(control.decode('utf-8'))
        if (architecture not in packages.keys()):
            packages[architecture] = {}
        if (package not in packages[architecture].keys()):
            packages[architecture][package] = {}
        if (version not in packages[architecture][package].keys()):
            packages[architecture][package][version] = []
        if (archive not in packages[architecture][package][version]):
            packages[architecture][package][version].append(archive)
    return packages

def print_packages(packages):
    for architecture in packages:
        print(architecture)
        for package in packages[architecture]:
            print('\t', package)
            for version in packages[architecture][package]:
                print('\t\t', version)
                for archive in packages[architecture][package][version]:
                    print('\t\t\t', archive)
            
def mark_outdated_packages(packages):
    outdated = []
    for architecture in packages:
        for package in packages[architecture]:
            if (len(packages[architecture][package]) == 1):
                continue
            max_version = sorted(packages[architecture][package].keys(), reverse=True)[0]
            for version in packages[architecture][package]:
                if (version != max_version):
                    for archive in packages[architecture][package][version]:
                        outdated.append(archive)
    return outdated
    
def help():
    print('Usage:')
    print('python cleanup_vcpkg_archive_cache.py <dir> <bucket> <access_key> <secret_key>')

if (len(sys.argv) != 5):
    help()
    sys.exit(1)

dir_name = sys.argv[1]
bucket_name = sys.argv[2]
access_key = sys.argv[3]
secret_key = sys.argv[4]

os.makedirs(dir_name, exist_ok=True)
s3.download_all(dir_name, bucket_name)
packages = get_packages(get_files(dir_name))
print_packages(packages)
outdated = mark_outdated_packages(packages)
s3.remove_files(outdated, bucket_name, access_key, secret_key)    
