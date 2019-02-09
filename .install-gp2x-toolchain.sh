#!/bin/sh

FILE="arm-open2x-linux-apps-gcc-4.1.1-glibc-2.3.6_i686_linux.tar.bz2.zip"
SHA256=ecb53e2799bbd6953621b2eedeed7280f5b03c3b2a6825607cafbe5dc1d545d8
URLALT="http://www.open2x.org/open2x/toolchains/$FILE"
URL="http://nanard.free.fr/grafx2/$FILE"

LIBS="open2x-libpack-20071903-gcc-4.1.1-glibc-2.3.6.tar.bz2.zip"
LIBSSHA256=31f46111c1d8bd38b720b292f65213adc624b050a614dab1199b56b35244efd7
LIBSURL="http://nanard.free.fr/grafx2/$LIBS"
LIBSURLALT="http://www.open2x.org/open2x/toolchains/$LIBS"

LIBS2=open2x-prerelease-libpack-20070307-gcc-4.1.1-glibc-2.3.6.zip
LIBS2SHA256=8eea174f7aa4ff174c488bf2d8419c7c9c03f999e3e4631b7c020dd311ef1199
LIBS2URL="http://nanard.free.fr/grafx2/$LIBS2"
LIBS2URLALT="http://www.open2x.org/open2x/toolchains/$LIBS2"

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
	echo "FAILED to check authenticity of GP2x toolchain archive"
	exit 1
fi

$WGET "$LIBSURL" || $WGET "$LIBSURLALT"
if ! echo "$LIBSSHA256  $LIBS" | shasum -c - ; then
	echo "FAILED to check authenticity of GP2x libpack archive"
	exit 1
fi

$WGET "$LIBS2URL" || $WGET "$LIBS2URLALT"
if ! echo "$LIBS2SHA256  $LIBS2" | shasum -c - ; then
	echo "FAILED to check authenticity of GP2x prerelease libpack archive"
	exit 1
fi

cd /
$SUDO tar xjf "${TMP}/${FILE}" || exit 1
echo "GP2x toolchain extracted to /opt/open2x"
cd /opt/open2x
$SUDO tar xjf "${TMP}/${LIBS}" || exit 1
echo "GP2x libpack extracted to /opt/open2x"
cd /opt/open2x/gcc-4.1.1-glibc-2.3.6
$SUDO unzip -o "${TMP}/${LIBS2}" || exit 1
rm -r ${TMP}
