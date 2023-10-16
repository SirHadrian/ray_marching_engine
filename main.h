#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

#ifndef EXIT_FAILURE
#define EXIT_FAILURE -1
#endif

#define WIDTH 1920.0f
#define HEIGHT 1080.0f

#define TITLE "OpenGL Template"

#define UNIFORM_TIME "u_time"
#define UNIFORM_MOUSE "u_mouse"
#define UNIFORM_RESOLUTION "u_resolution"

#define VERTEX_SHADER_PATH "shaders/vertex_shader.c"
#define FRAGMENT_SHADER_PATH "shaders/fragment_shader.c"

#define MAJOR_VERS 4
#define MINOR_VERS 6

typedef unsigned int uint;
typedef unsigned char uchar;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
static void cursor_position_callback(GLFWwindow *window, double xPos, double yPos);
void cursor_enter_callback(GLFWwindow *window, int inside);
void process_input(GLFWwindow *window, GLuint *shader_program);
char* get_shader(char *shader_file);
void die(const char *error);
void compile_shaders(const GLuint *const shader_program);
