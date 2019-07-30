#!/bin/sh


FILE="devkitpro-pacman.deb"
SHA256=aa674feee79b00948154ba132ed36266e1ace9fe1974c430764449ea26ac1d7a
URL="https://github.com/devkitPro/pacman/releases/download/devkitpro-pacman-1.0.1/$FILE"
URLALT="https://github.com/devkitPro/pacman/releases/download/devkitpro-pacman-1.0.1/$FILE"

SUDO=$(which sudo || echo "")
if [ -z "$SUDO" ] ; then
  echo "no sudo"
else
  echo "sudo: $SUDO"
fi

WGET=$(WGET=`which wget` ; \
       if  [ "$?" = "0" ] && [ -x "$WGET" ] ; \
       then echo "$WGET -nv" ; \
       else echo 'curl -R -O -L' ; fi)

TMP=$(mktemp -d)
cd ${TMP}

$WGET "$URL" || $WGET "$URLALT"
if ! echo "$SHA256  $FILE" | shasum -c - ; then
	echo "FAILED to check authenticity of devikpro dkp-pacman deb package"
	exit 1
fi

cd /
$SUDO dpkg -i "${TMP}/${FILE}" || exit 1
echo "devkitpro pacman installed"
yes Y | $SUDO dkp-pacman -S devkitA64 devkit-env libnx switch-tools switch-pkg-config switch-sdl2 switch-sdl2_ttf switch-sdl2_image switch-zlib switch-bzip2 switch-libpng switch-libjpeg-turbo switch-freetype || exit 1
echo "Switch toolchain and library extracted to /opt/devkitpro.."
rm -r ${TMP}
