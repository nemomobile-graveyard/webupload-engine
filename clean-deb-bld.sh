qmake
make clean

\rm -rf \
    debian/files                \
    debian/tmp                  \
    debian/webupload-engine/         \
    debian/webupload-engine-dbg/     \
    debian/webupload-engine-dev/     \
    debian/webupload-engine-doc/     \
    debian/webupload-engine-tests/   \
    debian/webupload-engine-utils/   \
    debian/webupload-engine-l10n-engineering-english/ \
    debian/*.substvars          \
    debian/*.debhelper

find . -name Makefile -exec \rm -rf {} \;
find . -name err -exec \rm -rf {} \;
find . -name moc -exec \rm -rf {} \;
find . -name obj -exec \rm -rf {} \;
find . -name out -exec \rm -rf {} \;
find . -name *~ -exec \rm -rf {} \;
find . -name *.gc* -exec \rm -rf {} \;

\rm -rf \
    build-stamp     \
    doc/           \
    config.* configure*

\rm -rf \
    sharing-ui-plugin/uploadengine_interface.*      \
    webupload-engine/transferengine_interface.*     \
    webupload-engine/uploadengine_adaptor.*         \
    webupload-tbr/accounts-mgr/accounts_adaptor.* \
    webupload-tbr/libaccounts-mgr/accounts_interface.*

\rm -rf \
    ../libwebupload*.deb \
    ../webupload-engine*.deb    \
    ../webupload-engine*.dsc    \
    ../webupload-engine*.tar.gz \
    ../webupload-engine*.changes 

