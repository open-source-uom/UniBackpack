#!/bin/bash
set -e

VERSION="1.0"
ARCH="amd64"
PACKAGE_NAME="unibackpack_${VERSION}_${ARCH}"

echo "Packaging UniBackpack v${VERSION}..."

# Clean previous package
rm -rf UniBackpack-deb
rm -f ${PACKAGE_NAME}.deb

# Create directory structure
mkdir -p UniBackpack-deb/DEBIAN
mkdir -p UniBackpack-deb/usr/bin
mkdir -p UniBackpack-deb/usr/share/applications
mkdir -p UniBackpack-deb/usr/share/icons/hicolor/256x256/apps

# Copy binary
cp build/UniBackpack UniBackpack-deb/usr/bin/unibackpack

# Copy icon if it exists
if [ -f resources/icons/unibackpack.png ]; then
    cp resources/icons/unibackpack.png UniBackpack-deb/usr/share/icons/hicolor/256x256/apps/unibackpack.png
fi

# Create control file
cat > UniBackpack-deb/DEBIAN/control << EOF
Package: unibackpack
Version: ${VERSION}
Section: utils
Priority: optional
Architecture: ${ARCH}
Depends: libqt6widgets6, libqt6core6, libqt6gui6, polkitd, pkexec
Maintainer: Apostolos Chalis <achalis@csd.auth.gr>
Description: UniBackpack - University software installer
 A Qt6 application that installs essential software packages
 for Greek university students based on their department.
EOF

# Create desktop entry
cat > UniBackpack-deb/usr/share/applications/unibackpack.desktop << EOF
[Desktop Entry]
Name=UniBackpack
Comment=University software installer
Exec=unibackpack
Icon=unibackpack
Terminal=false
Type=Application
Categories=Utility;
EOF

# Build the .deb
dpkg-deb --build UniBackpack-deb ${PACKAGE_NAME}.deb

echo "Package created: ${PACKAGE_NAME}.deb"
echo "Install with: sudo apt install ./${PACKAGE_NAME}.deb"