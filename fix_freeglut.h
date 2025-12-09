// fix_freeglut.h  –  put this file in your project folder
#pragma once

// Force the modern exit declaration BEFORE freeglut gets a chance
#include <stdlib.h>

// Now include freeglut – its weak "void exit(int);" is ignored
#include <glut.h>

// Kill the evil macro that freeglut sometimes adds
#ifdef exit
#undef exit
#endif