#!/bin/sh
bindir=$(pwd)
cd /home/aweyu/HAI819I_ProjetMoteur/TP5_code/TP1/
export 

if test "x$1" = "x--debugger"; then
	shift
	if test "xYES" = "xYES"; then
		echo "r  " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		/usr/bin/gdb -batch -command=$bindir/gdbscript --return-child-result /home/aweyu/HAI819I_ProjetMoteur/TP5_code/build/TP1 
	else
		"/home/aweyu/HAI819I_ProjetMoteur/TP5_code/build/TP1"  
	fi
else
	"/home/aweyu/HAI819I_ProjetMoteur/TP5_code/build/TP1"  
fi
