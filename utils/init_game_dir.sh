#!/bin/bash
# $1: name of game
if [ $# -eq 0 ]; then
	echo "You're supposed to call this like init_game [GAME_NAME].";
	exit;
fi
mkdir "$1"; cd "$1";
mkdir images audio text scenes sprites react;
echo "" | tee data.bin main.c db.json # add more files here as you come up with them
# Make DB files. 
echo "" | tee images/db_img, audio/db_audio, text/db_text, scenes/db_scene, scenes/db_posori, scenes/db_exit, react/db_react; 
# Make JSON equivalents of vanilla DB files for editability.
echo "" | tee images/db_img.json, audio/db_audio.json, text/db_text.json, scenes/db_scene.json, scenes/db_posori.json, scenes/db_exit.json, react/db_react.json; 

