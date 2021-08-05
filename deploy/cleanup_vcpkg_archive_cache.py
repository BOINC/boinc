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

def get_hash_from_name(name):
    parts = name.split('-')
    return parts[2].split('.')[0]

def read_vcpkg_abi_info_content(content, packages):
    dependencies = []
    lines = content.split('\n')
    for line in lines:
        if line:
            pair = line.split(' ')
            if (pair[0] in packages):
                dependencies.append(pair[1])
    return dependencies

def read_vcpkg_abi_info(archive, package, packages):
    zip_file = zipfile.ZipFile(archive, 'r')
    if (package == 'gtest'):
        package = 'GTest'
    file_name = 'share/'+package+'/vcpkg_abi_info.txt'
    try:
        info_file = zip_file.read(file_name)
        return read_vcpkg_abi_info_content(info_file.decode('utf-8'), packages)
    except Exception as ex:
        print('Failed to read the file', file_name, 'from', archive, ':', ex)
        return ''

def mark_outdated_packages(packages, modification_dates):
    outdated = []
    for architecture in packages:
        for package in packages[architecture]:
            archives_with_same_version = {}
            max_version = sorted(packages[architecture][package].keys(), reverse=True)[0]
            for version in packages[architecture][package]:
                if (version != max_version):
                    for archive in packages[architecture][package][version]:
                        outdated.append(archive)
                else:
                    if (len(packages[architecture][package][version]) == 1):
                        continue
                    for archive in packages[architecture][package][version]:
                        if (modification_dates[archive] not in archives_with_same_version and archive in modification_dates):
                            archives_with_same_version[modification_dates[archive]] = archive
                    max_date = sorted(archives_with_same_version.keys(), reverse=True)[0]
                    for archive in packages[architecture][package][version]:
                        if (archive != archives_with_same_version[max_date]):
                            outdated.append(archive)
    return outdated

def get_hash_list(packages):
    hash_list = []
    for architecture in packages:
        for package in packages[architecture]:
            for version in packages[architecture][package]:
                for archive in packages[architecture][package][version]:
                  hash_list.append(get_hash_from_name(os.path.basename(archive)))
    return hash_list

def remove_outdated_from_hash_list(hash_list, outdated):
    for package in outdated:
        package_hash = get_hash_from_name(os.path.basename(package))
        if (package_hash in hash_list):
            hash_list.remove(package_hash)

def add_package_to_outdated_by_hash(packages, outdated, package_hash, architecture):
    for package in packages[architecture]:
        for version in packages[architecture][package]:
            for archive in packages[architecture][package][version]:
                if (get_hash_from_name(os.path.basename(archive)) == package_hash and archive not in outdated):
                    outdated.append(archive)
                    return

def process_package_dependencies(package_hash, dependencies, packages, outdated, hash_list, architecture):
    is_valid = True
    if (package_hash not in hash_list):
        add_package_to_outdated_by_hash(packages, outdated, package_hash, architecture)
        is_valid = False
    if (package_hash not in dependencies):
        return is_valid
    package_dependencies = dependencies[package_hash]
    for dependency_hash in package_dependencies:
        is_valid = is_valid and process_package_dependencies(dependency_hash, dependencies, packages, outdated, hash_list, architecture)
    if (not is_valid):
        add_package_to_outdated_by_hash(packages, outdated, package_hash, architecture)
        if (package_hash in hash_list):
            hash_list.remove(package_hash)
        return False
    return is_valid

def process_dependencies_list(dependencies, packages, outdated, hash_list, architecture):
    for package_hash in dependencies:
        process_package_dependencies(package_hash, dependencies, packages, outdated, hash_list, architecture)

def mark_duplicate_packages(packages, outdated):
    hash_list = get_hash_list(packages)
    remove_outdated_from_hash_list(hash_list, outdated)

    for architecture in packages:
        dependencies_list = {}
        for package in packages[architecture]:
            for version in packages[architecture][package]:
                for archive in packages[architecture][package][version]:
                    dependencies = read_vcpkg_abi_info(archive, package, packages[architecture].keys())
                    if (len(dependencies) != 0):
                        dependencies_list[get_hash_from_name(os.path.basename(archive))] = dependencies
        process_dependencies_list(dependencies_list, packages, outdated, hash_list, architecture)

def print_outdated(outdated, packages):
    for architecture in packages:
        for package in packages[architecture]:
            for version in packages[architecture][package]:
                for archive in packages[architecture][package][version]:
                    if (archive in outdated):
                        print(architecture, package, version, archive, sep=' -> ')

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

modification_dates = s3.download_all(dir_name, bucket_name)
packages = get_packages(get_files(dir_name))
print_packages(packages)
outdated = mark_outdated_packages(packages, modification_dates)

mark_duplicate_packages(packages, outdated)
print('Outdated packages:')
print_outdated(outdated, packages)
s3.remove_files(outdated, bucket_name, access_key, secret_key)
