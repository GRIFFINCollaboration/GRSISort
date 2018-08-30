# Run this script to help report Errors in GRSISort
# Author by Ryan Dunlop, 22/10/2015

# taken from stackexchange (https://unix.stackexchange.com/questions/6345/how-can-i-get-distribution-name-and-version-number-in-a-simple-shell-script)
if [ -f /etc/os-release ]; then
	# freedesktop.org and systemd
	. /etc/os-release
	OS=$NAME
	VER=$VERSION_ID
elif type lsb_release >/dev/null 2>&1; then
	# linuxbase.org
	OS=$(lsb_release -si)
	VER=$(lsb_release -sr)
elif [ -f /etc/lsb-release ]; then
	# For some versions of Debian/Ubuntu without lsb_release command
	. /etc/lsb-release
	OS=$DISTRIB_ID
	VER=$DISTRIB_RELEASE
elif [ -f /etc/debian_version ]; then
	# Older Debian/Ubuntu/etc.
	OS=Debian
	VER=$(cat /etc/debian_version)
elif [ -f /etc/SuSe-release ]; then
	# Older SuSE/etc.
	OS=SuSe
	VER=unknown
elif [ -f /etc/redhat-release ]; then
	# Older Red Hat, CentOS, etc.
	OS=$(cat /etc/redhat-release | awk '{print $1}')
	VER=$(cat /etc/redhat-release | awk '{print $3}')
else
	# Fall back to uname, e.g. "Linux <version>", also works for BSD, etc.
	OS=$(uname -s)
	VER=$(uname -r)
fi

echo "HOSTNAME     = `hostname`"
echo "SYSTEM       = `uname`"
echo "OS           = $OS"
echo "VER          = $VER"
echo "GRSISYS      = $GRSISYS"
echo "ROOTSYS      = $ROOTSYS"

echo ROOT Version = `root-config --version`
printf "\nComputer and Path to File that failed: \n\n"

LASTDIR=$PWD
cd $GRSISYS
echo GRSISort Branch = `git rev-parse --abbrev-ref HEAD`
printf "\nLast Commit: " 

git log -1

cd $LASTDIR
