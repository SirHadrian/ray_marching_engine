#include "main.h"

// Cursor state
double xMousePos, yMousePos = 0.f;
int inWindow = FALSE;

int main() {

  if (!glfwInit()) {
    die("Could not initialize GLFW");
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, MAJOR_VERS);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, MINOR_VERS);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, TITLE, NULL, NULL);

  if (!window) {
    glfwTerminate();
    die("Failed to create GLFW window");
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    die("Failed to initialize GLAD");
  }

  glViewport(0, 0, WIDTH, HEIGHT);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // Mouse
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  glfwSetCursorEnterCallback(window, cursor_enter_callback);

  float vertices[] = {
      1.0f,  1.0f,  0.0f, // top right
      1.0f,  -1.0f, 0.0f, // bottom right
      -1.0f, -1.0f, 0.0f, // bottom left
      -1.0f, 1.0f,  0.0f  // top left
  };

  unsigned int indices[] = {
      // note that we start from 0!
      0, 1, 3, // first triangle
      1, 2, 3  // second triangle
  };

  GLuint VAO, VBO, EBO;

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
  //                       (void *)(3 * sizeof(float)));
  // glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindVertexArray(0);

  // Shader program
  GLuint shader_program = glCreateProgram();
  compile_shaders(&shader_program);

  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // Render loop
  while (!glfwWindowShouldClose(window)) {

    // Input
    process_input(window, &shader_program);

    // Render
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    float time = glfwGetTime();
    GLuint u_time_location = glGetUniformLocation(shader_program, UNIFORM_TIME);
    GLuint u_resolution_location =
        glGetUniformLocation(shader_program, UNIFORM_RESOLUTION);
    GLuint u_mouse_location =
        glGetUniformLocation(shader_program, UNIFORM_MOUSE);

    glUseProgram(shader_program);
    glUniform1f(u_time_location, time);
    glUniform2f(u_resolution_location, WIDTH, HEIGHT);
    glUniform2f(u_mouse_location, xMousePos, yMousePos);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    // glDrawArrays(GL_TRIANGLES, 0, 3);
    // glBindVertexArray(0);

    // Swap buffers and pull IO events
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Dealocate resources
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);

  glDeleteProgram(shader_program);

  glfwTerminate();

  return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void process_input(GLFWwindow *window, GLuint *shader_program) {

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) ||
      glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, TRUE);
  else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {

    glDeleteProgram(*shader_program);
    *shader_program = glCreateProgram();

    compile_shaders(shader_program);
  }
}

char *get_shader(char *shader_file) {

  FILE *file = fopen(shader_file, "r");
  if (!file) {
    die("Could not open the shader file");
  }

  fseek(file, 0, SEEK_END);
  unsigned int length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *shader_string = (char *)malloc(sizeof(char) * (length + 1));
  if (!shader_string) {
    die("Could not alocate memory for file contents");
  }

  char cursor;
  unsigned int index = 0;

  while ((cursor = fgetc(file)) != EOF) {
    shader_string[index] = cursor;
    index++;
  }

  shader_string[index] = '\0';

  fclose(file);

  return shader_string;
}

void die(const char *error) {
  fprintf(stderr, "%s\n", error);
  exit(EXIT_FAILURE);
}

void compile_shaders(const GLuint *const shader_program) {

  char *vertex_shader_source = get_shader(VERTEX_SHADER_PATH);

  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, (char const *const *)&vertex_shader_source,
                 NULL);
  glCompileShader(vertex_shader);

  int success;
  char info_log[512];
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);

  if (!success) {
    glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
    fprintf(stderr, "Vertex shader compilation error: %s\n", info_log);
    return;
  }

  free(vertex_shader_source);

  char *fragment_shader_source = get_shader(FRAGMENT_SHADER_PATH);

  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1,
                 (char const *const *)&fragment_shader_source, NULL);
  glCompileShader(fragment_shader);

  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);

  if (!success) {
    glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
    fprintf(stderr, "Fragment shader compilation error: %s\n", info_log);
    return;
  }

  free(fragment_shader_source);

  glAttachShader(*shader_program, vertex_shader);
  glAttachShader(*shader_program, fragment_shader);
  glLinkProgram(*shader_program);

  glGetProgramiv(*shader_program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(*shader_program, 512, NULL, info_log);
    fprintf(stderr, "Shader program linking error: %s\n", info_log);
    return;
  }

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
}
static void cursor_position_callback(GLFWwindow *window, double xPos,
                                     double yPos) {
  if (inWindow) {
    xMousePos = xPos;
    yMousePos = yPos;
  }
}

void cursor_enter_callback(GLFWwindow *window, int inside) {
  if (inside) {
    inWindow = TRUE;
  } else {
    inWindow = FALSE;
  }
}
