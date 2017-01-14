## Welcome to the FreeNAS 10 VM tools repo

In this repo, you will find an application that you can install on a Linux or
BSD OS that will allow you to monitor guest information like IP address and
health from the host to assist you in managing your VMs on your FreeNAS box.

Included in this document are instructions on how to build and install the tools on
various guest operating systems.

# To install on Arch linux:

* First, install development and required libraries:
```
sudo pacman -S base-devel git cmake binutils clang libmariadbclient libtool unixodbc
```

* Next, install poco:

``` 
git clone https://aur.archlinux.org/poco.git
cd poco
makepkg
sudo pacman -U poco-1.7.7-1-x86_64.pkg.tar.xz
```

* Then build freenas-vm-tools:
```
git clone https://github.com/freenas/freenas-vm-tools
mkdir build
cd build
export CXX=clang++
cmake ../
make
sudo make install
sudo cp ../systemd/freenas-vm-tools.service /usr/lib/systemd/system
sudo systemctl enable freenas-vm-tools
sudo systemctl start freenas-vm-tools
```

* Now the VM tools should be running on your Arch linux guest and you can monitor them from FreeNAS.

```
unix::>vm arch guest_info
 Property         Description                            Value                     
load_avg     Load average            0.0,0.0,0.0                                   
interfaces   Network configuration   fe80::5d64:ab74:c18f:2522%enp0s1,10.250.0.208 
```
