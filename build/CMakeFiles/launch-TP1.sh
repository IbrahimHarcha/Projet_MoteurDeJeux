#!/bin/sh
bindir=$(pwd)
cd /home/e20190004357/M1/semestre2/Moteur/Projet_MoteurDeJeux-master/TP1/
export 

if test "x$1" = "x--debugger"; then
	shift
	if test "xYES" = "xYES"; then
		echo "r  " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		/usr/bin/gdb -batch -command=$bindir/gdbscript --return-child-result /home/e20190004357/M1/semestre2/Moteur/Projet_MoteurDeJeux-master/build/TP1 
	else
		"/home/e20190004357/M1/semestre2/Moteur/Projet_MoteurDeJeux-master/build/TP1"  
	fi
else
	"/home/e20190004357/M1/semestre2/Moteur/Projet_MoteurDeJeux-master/build/TP1"  
fi
