#!/bin/bash

set -e

if [ -z "$GPG_AGENT_INFO" ] ; then
	echo "Press enter to continue if gpg-agent is running,"
	echo "Otherwise, run:"
	echo
	echo "      eval \$(gpg-agent --daemon)"
	echo
	echo "to preserve your sanity."
	read
fi

# Work inside the build directory, so it doesn't show up in Packages pathnames
cd bmbuild

# Build Packages and Contents, for all distros, per arch
for arch in i386 amd64 ; do
	for dir in $(find dists -name "binary-$arch" -type d -print) ; do
		apt-ftparchive packages $dir | tee $dir/Packages | gzip -9c > $dir/Packages.gz
		apt-ftparchive contents $dir | gzip -9c > $dir/../../Contents-$arch.gz
	done
done

# Build signed Release files for all debian distros
# Make sure you have gpg-agent running, or this will be a pain...
cd dists
for dir in * ; do
	if [ -f ../../apt/$dir.conf ] ; then
		(cd $dir && apt-ftparchive -c ../../../apt/$dir.conf release . > Release)
		gpg --use-agent --default-key B6C2250E --armor \
			--output $dir/Release.gpg \
			--detach-sign $dir/Release
	else
		echo "WARNING: conf file for $dir not found under apt/"
	fi
done
