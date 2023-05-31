#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-
export PATH=$PATH:/home/bruno/Desktop/gcc-arm-10.2-2020.11-x86_64-aarch64-none-linux-gnu/bin
ASSINGMENT_DIR=$(pwd)

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}
if [ ! -d ${OUTDIR} ]; then
    echo "Failed to create dir ${OUTDIR}"
    exit 1
fi

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper  #deep clean
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig #defconfig
    make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all   #build all
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs      #device tree
fi

echo "Adding the Image in outdir"
cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}
echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
    mkdir -p rootfs
    cd rootfs
    mkdir -p bin
    mkdir -p dev
    mkdir -p etc
    mkdir -p home
    mkdir -p lib
    mkdir -p lib64
    mkdir -p proc
    mkdir -p sys
    mkdir -p sbin
    mkdir -p tmp
    mkdir -p usr
    mkdir -p usr/sbin
    mkdir -p usr/bin
    mkdir -p usr/lib
    mkdir -p var
    mkdir -p var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox

else
    cd busybox
fi

# TODO: Make and install busybox
    make distclean
    make defconfig
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

cd ${OUTDIR}/rootfs
echo "Library dependencies"
SYSROOT=$(${CROSS_COMPILE}gcc --print-sysroot)
read PROG_INTER < <(${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter")
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library" | while read -r lib ; do
    lib=$(echo ${lib} | cut -d "[" -f2 | cut -d "]" -f1)
    #echo $lib
    #lib= ${lib} | cut -d "[" -f2 | cut -d "]" -f1
    cp ${SYSROOT}/lib64/${lib} ${OUTDIR}/rootfs/lib64
done
# TODO: Add library dependencies to rootfs
#Parse program intepreter:
PROG_INTER="${PROG_INTER#*\/}"
PROG_INTER="${PROG_INTER%\]*}"
echo $PROG_INTER
cp ${SYSROOT}/${PROG_INTER} ${OUTDIR}/rootfs/lib/

# TODO: Make device nodes
sudo mknod -m 666 ${OUTDIR}/rootfs/dev/null c 1 3
sudo mknod -m 666 ${OUTDIR}/rootfs/dev/console c 5 1
# TODO: Clean and build the writer utility
cd ${ASSINGMENT_DIR}
make clean
make CROSS_COMPILE=${CROSS_COMPILE}
# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp autorun-qemu.sh ${OUTDIR}/rootfs/home
cp finder-test.sh ${OUTDIR}/rootfs/home
cp writer.sh ${OUTDIR}/rootfs/home
cp writer ${OUTDIR}/rootfs/home
cp finder.sh ${OUTDIR}/rootfs/home
mkdir -p ${OUTDIR}/rootfs/home/conf
cp conf/username.txt ${OUTDIR}/rootfs/home/conf
cp conf/assignment.txt ${OUTDIR}/rootfs/home/conf
# TODO: Chown the root directory
sudo chown root:root ${OUTDIR}/rootfs
# TODO: Create initramfs.cpio.gz
cd ${OUTDIR}/rootfs
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
gzip -f ${OUTDIR}/initramfs.cpio
