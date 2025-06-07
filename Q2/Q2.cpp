#define _CRT_SECURE_NO_WARNINGS

#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/freeglut.h>

#define GLFW_INCLUDE_GLU
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <iostream>

#include "load_mesh.h"
#include "frame_timer.h"

using namespace glm;

int Width = 512;
int Height = 512;

GLuint VAO;
std::vector<GLuint> GLBuffers; // 0=VBO_pos, 1=VBO_norm, 2=unused, 3=EBO

mat4 modelingMatrix = mat4{
     10.0f,   0.0f,   0.0f,   0.0f,
      0.0f,  10.0f,   0.0f,   0.0f,
      0.0f,   0.0f,  10.0f,   0.0f,
      0.1f,  -1.0f,  -1.5f,   1.0f
};

mat4 modeling_inv_tr = transpose(inverse(modelingMatrix));

mat4 cameraMatrix = lookAt(
    vec3(0.0f, 0.0f, 0.0f),
    vec3(0.0f, 0.0f, -1.0f),
    vec3(0.0f, 1.0f, 0.0f)
);

mat4 projectionMatrix = frustum(-0.1f, 0.1f, -0.1f, 0.1f, 0.1f, 1000.0f);

float ka[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float kd[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float ks[] = { 0.0f, 0.0f, 0.0f, 1.0f };
float p = 0.0f; // p=0

vec3 lightDir = normalize(vec3(1.0f, 1.0f, 1.0f));
float l[]  = { lightDir.x, lightDir.y, lightDir.z, 0.0f };
float Ia[] = { 0.2f, 0.2f, 0.2f, 0.0f };
float la[] = { 0.0f, 0.0f, 0.0f, 0.0f };
float ld[] = { 1.0f, 1.0f, 1.0f, 0.0f };
float ls[] = { 0.0f, 0.0f, 0.0f, 0.0f };

void resize_callback(GLFWwindow*, int nw, int nh)
{
    Width = nw;
    Height = nh;
    // Tell the viewport to use all of our screen estate
    glViewport(0, 0, nw, nh);
}

void processInput(GLFWwindow* window)
{
    // Close when the user hits 'q' or escape
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS
        || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}


void reshape(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-0.1, 0.1, -0.1, 0.1, 0.1, 1000);

    glViewport(0, 0, Width, Height);

    glutPostRedisplay();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    start_timing();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(
        0, 0, 0,
        0, 0, -1,
        0, 1, 0
    );

    glTranslatef(0.1, -1, -1.5);
    glScalef(10, 10, 10);

    glBindVertexArray(VAO);
    GLsizei indexCount = (GLsizei)(gTriangles.size() * 3);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);

    float timeElapsed = stop_timing();
    gTotalFrames++;
    gTotalTimeElapsed += timeElapsed;
    float fps = gTotalFrames / gTotalTimeElapsed;
    char string[1024] = { 0 };
    sprintf(string, "OpenGL Bunny: %0.2f FPS", fps);
    glutSetWindowTitle(string);

    glutPostRedisplay();
    glutSwapBuffers();
}

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(Width, Height);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("OpenGL");
    load_mesh("bunny.obj");

    if (glewInit() != GLEW_OK)
    {
        std::cerr << "[FATAL] Error initializing glew!" << '\n';
        return -1;
    }

    glGenVertexArrays(1, &VAO);

    const int numBuffers = 4; // Buffers for Positions, Normals, None, Indices
    GLBuffers.resize(numBuffers, 0);
    glGenBuffers(numBuffers, &GLBuffers[0]);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, GLBuffers[0]);
    glBufferData(
        GL_ARRAY_BUFFER,
        gPositions.size() * sizeof(gPositions[0]), 
        &gPositions[0], 
        GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);


    glBindBuffer(GL_ARRAY_BUFFER, GLBuffers[1]);
    glBufferData(
        GL_ARRAY_BUFFER, 
        gNormals.size() * sizeof(gNormals[0]), 
        &gNormals[0],
        GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    std::vector<GLuint> flatIndices;
    flatIndices.reserve(gTriangles.size() * 3);

    for (size_t i = 0; i < gTriangles.size(); ++i) {
        flatIndices.push_back(gTriangles[i].indices[0]);
        flatIndices.push_back(gTriangles[i].indices[1]);
        flatIndices.push_back(gTriangles[i].indices[2]);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GLBuffers[3]);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        flatIndices.size() * sizeof(flatIndices[0]),
        flatIndices.data(),
        GL_STATIC_DRAW
    );


    glDisable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    //glShadeModel(GL_PHONG_WIN);

    glMaterialfv(GL_FRONT, GL_AMBIENT, ka);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, kd);
    glMaterialfv(GL_FRONT, GL_SPECULAR, ks);
    glMaterialf(GL_FRONT, GL_SHININESS, p);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, Ia);

    glLightfv(GL_LIGHT0, GL_POSITION, l);
    glLightfv(GL_LIGHT0, GL_AMBIENT, la);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, ld);
    glLightfv(GL_LIGHT0, GL_SPECULAR, ls);

    glutReshapeFunc(reshape);
    glutDisplayFunc(display);

    init_timer();
    glutMainLoop();
    return 0;
}