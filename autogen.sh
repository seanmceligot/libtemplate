set -x
if [ -f "Makefile" ]; then
	make clean maintainer-clean
fi
aclocal
autoconf
autoheader
automake --gnu
