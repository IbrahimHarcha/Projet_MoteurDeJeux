#!/bin/sh
bindir=$(pwd)
cd /home/aweyu/Moteur_de_jeux-Projet/Moteur_de_jeux-Projet/Projet_code/TP1/
export 

if test "x$1" = "x--debugger"; then
	shift
	if test "xYES" = "xYES"; then
		echo "r  " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		/usr/bin/gdb -batch -command=$bindir/gdbscript --return-child-result /home/aweyu/Moteur_de_jeux-Projet/Moteur_de_jeux-Projet/Projet_code/build/TP1 
	else
		"/home/aweyu/Moteur_de_jeux-Projet/Moteur_de_jeux-Projet/Projet_code/build/TP1"  
	fi
else
	"/home/aweyu/Moteur_de_jeux-Projet/Moteur_de_jeux-Projet/Projet_code/build/TP1"  
fi
