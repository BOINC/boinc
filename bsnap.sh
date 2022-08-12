sudo snap remove boinc
rm -rf *.snap
snapcraft snap --destructive-mode --output boinc_amd64.snap
sudo snap install --devmode boinc_amd64.snap
unsquashfs -f -d 3rdParty/buildCache/linux boinc_amd64.snap wxwidgets
boinc --version