# Maintainer: Apostolos Chalis <achalis@csd.auth.gr>
pkgname=unibackpack
_pkgname=UniBackpack
pkgver=1.1.0.r14.g358c0a2
pkgrel=1
pkgdesc="A lightweight Qt C++ application to automate software toolchain installation for university students"
arch=('x86_64')
url="https://github.com/open-source-uom/UniBackpack"
license=('GPL3')
depends=('qt6-base' 'polkit')
makedepends=('git' 'cmake' 'qt6-tools' 'gcc')
source=("git+https://github.com/open-source-uom/UniBackpack.git")
sha256sums=('SKIP')

pkgver() {
  cd "$_pkgname"
  if git describe --long --tags >/dev/null 2>&1; then
    git describe --long --tags | sed 's/^v//;s/\([^-]*-g\)/r\1/;s/-/./g'
  else
    printf "0.1.r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short=7 HEAD)"
  fi
}

build() {
  cmake -B build -S "$_pkgname" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr

  cmake --build build
}

package() {
  DESTDIR="$pkgdir" cmake --install build

  install -d "$pkgdir/usr/share/applications"
  install -d "$pkgdir/usr/share/pixmaps"

  if [ -f "$srcdir/$_pkgname/resources/icons/unibackpack.png" ]; then
    install -Dm644 "$srcdir/$_pkgname/resources/icons/unibackpack.png" "$pkgdir/usr/share/pixmaps/unibackpack.png"
    ICON="unibackpack"
  else
    ICON="system-software-install"
  fi

  cat <<EOF > "$pkgdir/usr/share/applications/unibackpack.desktop"
[Desktop Entry]
Type=Application
Version=1.0
Name=UniBackpack
GenericName=Software Toolchain Installer
Comment=Automate software toolchain installation for university students
Exec=unibackpack
Icon=$ICON
Terminal=false
StartupNotify=true
Categories=Development;Education;System;
EOF
}
