#! /bin/bash

g++ src/game.cpp -o build/game \
-lglfw -lGLEW -lGLU -lGL \
-I/home/scallywag/game/external/glfw-3.1.2/include/GLFW/ \
-I/home/scallywag/game/external/glm-0.9.7.1/ \
-I/home/scallywag/game/common/ \
-I/home/scallywag/game/src/ \
\
-L/home/scallywag/game/external/glfw-3.1.2/include/GLFW/ \
-L/home/scallywag/game/external/glm-0.9.7.1/ \
-L/home/scallywag/game/common/ \
-L/home/scallywag/game/src/ \

cd build/

./game >> /dev/tty
