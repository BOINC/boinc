import json
import os
import sys

def get_vcpkg_ports():
    vcpkg_ports = []
    for root, _, files in os.walk('3rdParty/vcpkg_ports/ports'):
        for file in files:
            if file == 'vcpkg.json':
                vcpkg_ports.append(os.path.join(root, file))
    return vcpkg_ports

def get_port_info(port):
    with open(port, 'r') as f:
        data = json.load(f)

    name = data.get('name')
    version = data.get('version')
    port_version = data.get('port-version', 0)

    return name, version, port_version

def parse_ports(file_path):
    with open(file_path, 'r') as f:
        data = json.load(f)

    ports_info = []
    for port, details in data.get('default', {}).items():
        baseline = details.get('baseline')
        port_version = details.get('port-version', 0)
        ports_info.append((port, baseline, port_version))

    return ports_info

def check_for_updated_ports():
    updated_ports = []
    for port in get_vcpkg_ports():
        name, version, port_version = get_port_info(port)
        for port, baseline, baseline_version in parse_ports('3rdParty/Windows/vcpkg/versions/baseline.json'):
            if name == port and (version != baseline or port_version != baseline_version):
                updated_ports.append((port, baseline, baseline_version))

    return updated_ports

updated_ports = check_for_updated_ports()
if (updated_ports):
    print('The following ports have been updated:')
    for port, baseline, baseline_version in updated_ports:
        print(f'{port}: {baseline}:{baseline_version}')
    sys.exit(1)
else:
    print('All ports are up-to-date.')
    sys.exit(0)
