# Maintainer: Apostolos Chalis <achalis@csd.auth.gr>
pkgname=unibackpack
_pkgname=UniBackpack
pkgver=1.1.0.r11.g0467eee
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
  # Generates version based on git tag or fallback to latest tag format
  git describe --long --tags 2>/dev/null | sed 's/^v//;s/\([^-]*-g\)/r\1/;s/-/./g' || echo "1.1.1"
}

build() {
  cmake -B build -S "$_pkgname" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr

  cmake --build build
}

package() {
  DESTDIR="$pkgdir" cmake --install build
}
