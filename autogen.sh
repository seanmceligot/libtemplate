set -x
if [ -f "Makefile" ]; then
	make clean maintainer-clean
fi
aclocal
#autoconf
autoreconf -i -f
autoheader
automake -a -c --gnu
