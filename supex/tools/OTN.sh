if [ -d $1 ];then
	export MYPATH=$1
else
	export MYFILE=$1
fi

luajit OTN.lua
