#!/bin/sh
set -ex

RELEASE=3.1.2

# Create release branch
git branch "v${RELEASE}"

# Create tmpdir
TMPDIR=$(mktemp -d)
cp -r . "${TMPDIR}/fossor-${RELEASE}"
cd "${TMPDIR}/fossor-${RELEASE}"

# Remove build leftovers etc
rm -rf src/*.o src/files.h src/filetypes.h src/bin2c src/fossor Makefile README README.options
# Remove things
rm -rf .git .gitignore .github .travis .travis.yml release.sh

# Change readme for devel
sed -i '/The master branch is rolling Development/d' README.md
sed -i "s/ DEVEL$/ ${RELEASE}/" README.md

# Create tarball(s)
cd "${TMPDIR}"
tar -czvf "fossor-${RELEASE}.tar.gz" "fossor-${RELEASE}"
tar -cjvf "fossor-${RELEASE}.tar.bz2" "fossor-${RELEASE}"
tar -cJvf "fossor-${RELEASE}.tar.xz" "fossor-${RELEASE}"

# Create binaries
cd "${TMPDIR}/fossor-${RELEASE}"

# x86_64-glibc
./configure --listener=none --os=linux --hostname=HOSTNAME
make
strip src/fossor
cp src/fossor "../fossor-${RELEASE}-x86_64-glibc"
make clean

# x86-64-static
export CFLAGS="-static"
export LDFLAGS="-static"
./configure --listener=none --os=linux --hostname=HOSTNAME
make
strip src/fossor
cp src/fossor "../fossor-${RELEASE}-x86_64-static"
make clean
