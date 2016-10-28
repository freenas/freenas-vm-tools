#!/bin/sh

PKGROOT=dest
BUILD_ROOT=../build
SOURCE_ROOT=..

build_deb_systemd()
{
    install -d ${PKGROOT}/DEBIAN
    install -d ${PKGROOT}/lib/systemd/system/
    install -d ${PKGROOT}/usr/local/sbin
    install -d ${PKGROOT}/usr/local/lib
    install -d ${PKGROOT}/usr/local/lib/freenas-vm-tools
    install deb/control ${PKGROOT}/DEBIAN/
    install ${BUILD_ROOT}/freenas-vm-tools ${PKGROOT}/usr/local/sbin/
    install ${BUILD_ROOT}/lib*.so ${PKGROOT}/usr/local/lib/freenas-vm-tools/
    install ${SOURCE_ROOT}/systemd/freenas-vm-tools.service ${PKGROOT}/lib/systemd/system/
    dpkg -b ${PKGROOT} freenas-vm-tools-systemd.deb
}

build_deb_systemd
