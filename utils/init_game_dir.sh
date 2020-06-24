#!/bin/bash
# $1: name of game
if [ $# -eq 0 ]; then
	echo "You're supposed to call this like init_game [GAME_NAME].";
	exit;
fi
mkdir "$1"; cd "$1";
echo "" | tee data.bin main.c # add more files here as you come up with them
echo "name:$1" > meta

