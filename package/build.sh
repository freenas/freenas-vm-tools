#!/bin/sh

PKGROOT=dest
BUILD_ROOT=../build
SOURCE_ROOT=..

build_deb_package()
{
    install -d ${PKGROOT}/DEBIAN
    install -d ${PKGROOT}/usr/local/sbin
    install -d ${PKGROOT}/usr/local/lib
    install -d ${PKGROOT}/usr/local/lib/freenas-vm-tools
    install deb/control ${PKGROOT}/DEBIAN/
    install ${BUILD_ROOT}/freenas-vm-tools ${PKGROOT}/usr/local/sbin/
    install ${BUILD_ROOT}/lib*.so ${PKGROOT}/usr/local/lib/freenas-vm-tools/

    if [ $1 = "systemd" ]; then
          install -d ${PKGROOT}/lib/systemd/system/ 
          install ${SOURCE_ROOT}/systemd/freenas-vm-tools.service ${PKGROOT}/lib/systemd/system/
	  dpkg -b ${PKGROOT} freenas-vm-tools-systemd.deb
	  rm -rf ${PKGROOT}

    elif [ $1 = "upstart" ]; then
          install -d ${PKGROOT}/etc/init
          install ${SOURCE_ROOT}/upstart/freenas-vm-tools.conf ${PKGROOT}/etc/init
	  dpkg -b ${PKGROOT} freenas-vm-tools-upstart.deb
	  rm -rf ${PKGROOT}

    fi
}

build_deb_package systemd

build_deb_package upstart
