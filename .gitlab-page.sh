#!/bin/sh
# (c) 2019 Thomas Bernard
#

OUTFILE=public/index.html
VERSION=$(cat VERSION)
DATE=$(date -R)
LINUX_JOB_ID=$(cat LINUX_JOB_ID)
WIN32_JOB_ID=$(cat WIN32_JOB_ID)
ATARI_JOB_ID=$(cat ATARI_JOB_ID)
GP2X_JOB_ID=$(cat GP2X_JOB_ID)
SWITCH_JOB_ID=$(cat SWITCH_JOB_ID)

# for debug:
echo "CI_JOB_URL=${CI_JOB_URL}"
echo "CI_PAGES_URL=${CI_PAGES_URL}"
echo "CI_PIPELINE_URL=${CI_PIPELINE_URL}"
echo "CI_PROJECT_URL=${CI_PROJECT_URL}"
echo "CI_PROJECT_NAMESPACE=${CI_PROJECT_NAMESPACE}"
echo "CI_PROJECT_NAME=${CI_PROJECT_NAME}"
echo "CI_PROJECT_PATH_SLUG=${CI_PROJECT_PATH_SLUG}"

echo "<!DOCTYPE html>" > ${OUTFILE}
echo "<html lang=\"en\">" >> ${OUTFILE}
echo "<head>" >> ${OUTFILE}
echo "<title>GrafX2</title>" >> ${OUTFILE}
echo "<link href=\"gfx2.css\" rel=\"stylesheet\" type=\"text/css\">" >> ${OUTFILE}
echo "<link href=\"gfx2.png\" rel=\"icon\" type=\"image/png\">" >> ${OUTFILE}
echo "</head>" >> ${OUTFILE}
echo "<body>" >> ${OUTFILE}
echo "<h1>GrafX2 version ${VERSION}</h1>" >> ${OUTFILE}
echo "<p>build date : ${DATE}</p>" >> ${OUTFILE}
echo "<ul>" >> ${OUTFILE}
echo "<li><a href=\"doxygen/html/\">doxygen</a></li>" >> ${OUTFILE}
echo "<li><a href=\"htmldoc/\">htmldoc</a></li>" >> ${OUTFILE}
echo "<li>Sources :" >> ${OUTFILE}
echo "<a href=\"${CI_PROJECT_URL}/-/jobs/${LINUX_JOB_ID}/artifacts/raw/grafx2-${VERSION}-src.tgz\">grafx2-${VERSION}-src.tgz</a></li>" >> ${OUTFILE}
if [ -n "${WIN32_JOB_ID}" ] ; then
echo "<li>Windows installer (win32 API) :" >> ${OUTFILE}
echo "<a href=\"${CI_PROJECT_URL}/-/jobs/${WIN32_JOB_ID}/artifacts/raw/install/grafx2-win32-${VERSION}.win32.exe\">grafx2-win32-${VERSION}.win32.exe</a></li>" >> ${OUTFILE}
echo "<li>Windows portable (win32 API) :" >> ${OUTFILE}
echo "<a href=\"${CI_PROJECT_URL}/-/jobs/${WIN32_JOB_ID}/artifacts/raw/grafx2-win32-${VERSION}-win32.zip\">grafx2-win32-${VERSION}-win32.zip</a></li>" >> ${OUTFILE}
echo "<li>Windows installer (SDL API) :" >> ${OUTFILE}
echo "<a href=\"${CI_PROJECT_URL}/-/jobs/${WIN32_JOB_ID}/artifacts/raw/install/grafx2-sdl-${VERSION}.win32.exe\">grafx2-sdl-${VERSION}.win32.exe</a></li>" >> ${OUTFILE}
echo "<li>Windows portable (SDL API) :" >> ${OUTFILE}
echo "<a href=\"${CI_PROJECT_URL}/-/jobs/${WIN32_JOB_ID}/artifacts/raw/grafx2-sdl-${VERSION}-win32.zip\">grafx2-sdl-${VERSION}-win32.zip</a></li>" >> ${OUTFILE}
echo "<li>Windows installer (SDL2 API) :" >> ${OUTFILE}
echo "<a href=\"${CI_PROJECT_URL}/-/jobs/${WIN32_JOB_ID}/artifacts/raw/install/grafx2-sdl2-${VERSION}.win32.exe\">grafx2-sdl2-${VERSION}.win32.exe</a></li>" >> ${OUTFILE}
echo "<li>Windows portable (SDL2 API) :" >> ${OUTFILE}
echo "<a href=\"${CI_PROJECT_URL}/-/jobs/${WIN32_JOB_ID}/artifacts/raw/grafx2-sdl2-${VERSION}-win32.zip\">grafx2-sdl2-${VERSION}-win32.zip</a></li>" >> ${OUTFILE}
fi
if [ -n "${ATARI_JOB_ID}" ] ; then
echo "<li>Atari (SDL API) :" >> ${OUTFILE}
echo "<a href=\"${CI_PROJECT_URL}/-/jobs/${ATARI_JOB_ID}/artifacts/raw/grafx2-sdl-${VERSION}-m68k-atari-mint.zip\">grafx2-sdl-${VERSION}-m68k-atari-mint.zip</a></li>" >> ${OUTFILE}
fi
if [ -n "${GP2X_JOB_ID}" ] ; then
echo "<li>GP2X (SDL API) :" >> ${OUTFILE}
echo "<a href=\"${CI_PROJECT_URL}/-/jobs/${GP2X_JOB_ID}/artifacts/raw/grafx2-sdl-${VERSION}-nottf-gp2x.zip\">grafx2-sdl-${VERSION}-nottf-gp2x.zip</a></li>" >> ${OUTFILE}
fi
if [ -n "${SWITCH_JOB_ID}" ] ; then
echo "<li>Switch (SDL2 API) :" >> ${OUTFILE}
echo "<a href=\"${CI_PROJECT_URL}/-/jobs/${SWITCH_JOB_ID}/artifacts/raw/grafx2-sdl2-${VERSION}-switch.zip\">grafx2-sdl2-${VERSION}-switch.zip</a></li>" >> ${OUTFILE}
fi
echo "</ul>" >> ${OUTFILE}
echo "<ul>" >> ${OUTFILE}

echo "<li><a href=\"http://grafx2.tk/\">Home page</a></li>" >> ${OUTFILE}
echo "<li>Source repository : <a href=\"${CI_PROJECT_URL}\">${CI_PROJECT_URL}</a></li>" >> ${OUTFILE}
echo "<li>Bug tracker : <a href=\"https://pulkomandy.tk/projects/GrafX2/query\">https://pulkomandy.tk/projects/GrafX2/query</a></li>" >> ${OUTFILE}
echo "<li>Open Hub : <a href=\"https://www.openhub.net/p/grafx2\">https://www.openhub.net/p/grafx2</a></li>" >> ${OUTFILE}
echo "</ul>" >> ${OUTFILE}
echo "<p>Copyright (C)1996-2001 Sunset Design (G.Dorme & K.Maritaud)<br>" >> ${OUTFILE}
echo "Copyright (C)2007-2019 the Grafx2 Project Team (A.Destugues & Y.Rizoud)</p>" >> ${OUTFILE}
echo "</body>" >> ${OUTFILE}
echo "</html>" >> ${OUTFILE}

echo ${VERSION}
