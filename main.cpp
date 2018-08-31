#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <cstdlib>
#include <unistd.h>
#include <cmath>
#include <png.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "kociemba/include/search.h"

using namespace std;
using namespace glm;

const int width = 1920;
const int height = 1080;
const bool fullscreen = true;
const float fov = 60.0f;
const float fps = 120.0f;
const float mouseSpeed = 0.003f;
const float maxDistanse = 10000.0f;
const float genSpeed = 20.0f;
const float solveSpeed = 1.5f;
const float freeSpeed = 50000.0f;
const float timePressKey = 0.5f;
const int countGenRotations = 100;

struct cube {
    char L='0',R='0',U='0',D='0',F='0',B='0';
} cubes[27];
GLFWwindow *window;
string currentRotate;
queue<string> rotations;
float rotAngle = 0.0f;
unsigned char argb_data[width*height*4], rgb_data[width*height*3];
float verticalAngle = 0.5f, horizontalAngle = 0.5f;
float curDistance = 8.0f;
mat4 projection, view, model, MVP;
int flag = 0;

const int dGU[] = {0,1,2,3,4,5,6,7,8};
const int dGD[] = {18,19,20,21,22,23,24,25,26};
const int dGL[] = {0,3,6,9,12,15,18,21,24};
const int dGR[] = {2,5,8,11,14,17,20,23,26};
const int dGF[] = {6,7,8,15,16,17,24,25,26};
const int dGB[] = {0,1,2,9,10,11,18,19,20};
const int gDSU[] = {0,1,2,3,4,5,6,7,8};
const int gDSR[] = {8,5,2,17,14,11,26,23,20};
const int gDSF[] = {6,7,8,15,16,17,24,25,26};
const int gDSD[] = {24,25,26,21,22,23,18,19,20};
const int gDSL[] = {0,3,6,9,12,15,18,21,24};
const int gDSB[] = {2,1,0,11,10,9,20,19,18};

void fillCubes() {
    for (int i : dGU) cubes[i].U = 'Y';
    for (int i : dGD) cubes[i].D = 'W';
    for (int i : dGL) cubes[i].L = 'O';
    for (int i : dGR) cubes[i].R = 'R';
    for (int i : dGF) cubes[i].F = 'B';
    for (int i : dGB) cubes[i].B = 'G';
}

void generateRotations(int count) {
    string tmp = "  ";
    const char dR[] = "UDLRBF";
    const char dD[] = "  ''2";
    for (int i = 0;i<count;++i) {
        tmp[0] = dR[rand()%6];
        tmp[1] = dD[rand()%5];
        rotations.push(tmp);
    }
    if (currentRotate=="") {
        currentRotate = rotations.front();
        rotations.pop();
    }
}

int init() {
    if (!glfwInit()) {
        cout<<"Error GLFW init"<<endl;
        return -1;
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(width, height, "Rubik's cube solver", (fullscreen)?glfwGetPrimaryMonitor():NULL, NULL);
    if (window == NULL) {
        cout<<"Error opening window"<<endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        cout<<"Error GLEW init"<<endl;
        return -1;
    }
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glClearColor(0.458f,0.733f, 0.992f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    //glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    return 0;
}

void saveImage(string filename, unsigned char *data) {
    FILE *fp = fopen(filename.c_str(), "wb");
    if (!fp) {
        cout<<"File save error"<<endl;
        return;
    }
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    png_init_io(png_ptr, fp);

    png_set_IHDR(png_ptr, info_ptr, width, height,
        8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);
    unsigned char *rows[height];
    for (int i = 0;i<height;++i)
        rows[i] = data + (i*width*3);
    png_set_rows(png_ptr, info_ptr, rows);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, nullptr);
    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, nullptr);
    fclose(fp);
}

GLuint loadBMP(const char * imagepath){
    unsigned char header[54];
    unsigned int dataPos;
    unsigned int imageSize;
    unsigned int width, height;
    unsigned char * data;
    FILE * file = fopen(imagepath,"rb");
    if (!file){
        printf("%s could not be opened\n", imagepath);
        getchar();
        return 0;
    }
    if ( fread(header, 1, 54, file)!=54 ){
        printf("Not a correct BMP file\n");
        fclose(file);
        return 0;
    }
    if ( header[0]!='B' || header[1]!='M' ){
        printf("Not a correct BMP file\n");
        fclose(file);
        return 0;
    }
    if ( *(int*)&(header[0x1E])!=0  )         {printf("Not a correct BMP file\n");    fclose(file); return 0;}
    if ( *(int*)&(header[0x1C])!=24 )         {printf("Not a correct BMP file\n");    fclose(file); return 0;}
    dataPos    = *(int*)&(header[0x0A]);
    imageSize  = *(int*)&(header[0x22]);
    width      = *(int*)&(header[0x12]);
    height     = *(int*)&(header[0x16]);
    if (imageSize==0)    imageSize=width*height*3;
    if (dataPos==0)      dataPos=54;
    data = new unsigned char [imageSize];
    fread(data,1,imageSize,file);
    fclose (file);

    for (int i = 0;i<width*height;++i) {
        unsigned char tmp = data[i*3];
        data[i*3] = data[i*3+1];
        data[i*3+1] = tmp;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    delete [] data;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    return textureID;
}

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open()){
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }
    else{
        printf("Impossible to open %s\n", vertex_file_path);
        getchar();
        return 0;
    }

    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }
    GLint Result = GL_FALSE;
    int InfoLogLength;

    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }

    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }

    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }
    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
    return ProgramID;
}

void createVertices(GLfloat *vertexbuffer, GLfloat *uvbuffer) {
    const GLfloat dxyz[] = {
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
    };
    const GLfloat duv[] {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
    };
    int id = 0;
    for (int y = 1;y>=-1;--y) {
        for (int z = -1;z<=1;++z) {
            for (int x = -1;x<=1;++x) {
                for (int i = 0;i<6;++i) {
                    for (int j = 0;j<6;++j) {
                        vertexbuffer[id*108+i*18+j*3] = dxyz[i*18+j*3] + x - 0.5f;
                        vertexbuffer[id*108+i*18+j*3+1] = dxyz[i*18+j*3+1] + y - 0.5f;
                        vertexbuffer[id*108+i*18+j*3+2] = dxyz[i*18+j*3+2] + z - 0.5f;
                        uvbuffer[id*72+i*12+j*2] = duv[j*2]*0.25f + 0.0225;
                        uvbuffer[id*72+i*12+j*2+1] = duv[j*2+1]*0.5f;
                        char color;
                        switch (i) {
                            case 0: color = cubes[id].D;break;
                            case 1: color = cubes[id].U;break;
                            case 2: color = cubes[id].L;break;
                            case 3: color = cubes[id].R;break;
                            case 4: color = cubes[id].B;break;
                            case 5: color = cubes[id].F;break;
                        }
                        switch (color) {
                            case '0':
                                uvbuffer[id*72+i*12+j*2] += 0.75f;
                                break;
                            case 'W':
                                uvbuffer[id*72+i*12+j*2] += 0.25f;
                                break;
                            case 'Y':
                                uvbuffer[id*72+i*12+j*2] += 0.5f;
                                break;
                            case 'R':
                                uvbuffer[id*72+i*12+j*2+1] += 0.5f;
                                break;
                            case 'G':
                                uvbuffer[id*72+i*12+j*2] += 0.25f;
                                uvbuffer[id*72+i*12+j*2+1] += 0.5f;
                                break;
                            case 'B':
                                uvbuffer[id*72+i*12+j*2] += 0.5f;
                                uvbuffer[id*72+i*12+j*2+1] += 0.5f;
                                break;
                        }

                    }
                }
                id++;
            }
        }
    }
}

void computeMatrices() {
    static bool first = true;
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    glfwSetCursorPos(window, width/2, height/2);
    if (first) {
        xpos = width/2;
        ypos = height/2;
        first = false;
    }
    horizontalAngle += mouseSpeed * float(width/2 - xpos );
    verticalAngle   -= mouseSpeed * float(height/2 - ypos );
    if (verticalAngle>1.57) verticalAngle = 1.57;
    if (verticalAngle<-1.57) verticalAngle = -1.57;
    while (horizontalAngle>6.282) {horizontalAngle -= 6.282;}
    vec3 pos(
        cos(verticalAngle) * sin(horizontalAngle),
        sin(verticalAngle),
        cos(verticalAngle) * cos(horizontalAngle)
    );
    view = lookAt(pos*curDistance ,vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    MVP = projection*view*model;
}

void rotateVertices(GLfloat *vertexbuffer, GLfloat *uvbuffer) {
    if (currentRotate == "") return;
    float angleX = 0.0f, angleY = 0.0f, angleZ = 0.0f;
    int ids[9];
    switch (currentRotate[0]) {
    case 'L':
        angleX = rotAngle;
        for (int i = 0;i<9;++i) ids[i] = dGL[i];
        break;
    case 'R':
        angleX = -rotAngle;
        for (int i = 0;i<9;++i) ids[i] = dGR[i];
        break;
    case 'D':
        angleY = rotAngle;
        for (int i = 0;i<9;++i) ids[i] = dGD[i];
        break;
    case 'U':
        angleY = -rotAngle;
        for (int i = 0;i<9;++i) ids[i] = dGU[i];
        break;
    case 'F':
        angleZ = rotAngle;
        for (int i = 0;i<9;++i) ids[i] = dGF[i];
        break;
    case 'B':
        angleZ = -rotAngle;
        for (int i = 0;i<9;++i) ids[i] = dGB[i];
        break;
    }
    for (int id = 0;id<9;++id) {
        int i = ids[id];
        for (int j = 0;j<36;++j) {
            float x = vertexbuffer[i*108+j*3];
            float y = vertexbuffer[i*108+j*3+1];
            float z = vertexbuffer[i*108+j*3+2];
            float nx = x*cos(radians(angleY)) + z*sin(radians(angleY));
            float ny = y;
            float nz = z*cos(radians(angleY)) - x*sin(radians(angleY));
            x = nx;
            y = ny*cos(radians(angleX)) - nz*sin(radians(angleX));
            z = nz*cos(radians(angleX)) + ny*sin(radians(angleX));
            vertexbuffer[i*108+j*3] = x*cos(radians(angleZ)) + y*sin(radians(angleZ));
            vertexbuffer[i*108+j*3+1] = y*cos(radians(angleZ)) - x*sin(radians(angleZ));
            vertexbuffer[i*108+j*3+2] = z;

        }
    }
}

cube rotateCube(cube curcube, char direction) {
    cube newcube;
    switch (direction) {
    case 'U':
        newcube.U = curcube.U;
        newcube.D = curcube.D;
        newcube.L = curcube.F;
        newcube.B = curcube.L;
        newcube.R = curcube.B;
        newcube.F = curcube.R;
        break;
    case 'D':
        newcube.U = curcube.U;
        newcube.D = curcube.D;
        newcube.F = curcube.L;
        newcube.L = curcube.B;
        newcube.B = curcube.R;
        newcube.R = curcube.F;
        break;
    case 'L':
        newcube.L = curcube.L;
        newcube.R = curcube.R;
        newcube.U = curcube.B;
        newcube.B = curcube.D;
        newcube.D = curcube.F;
        newcube.F = curcube.U;
        break;
    case 'R':
        newcube.L = curcube.L;
        newcube.R = curcube.R;
        newcube.B = curcube.U;
        newcube.D = curcube.B;
        newcube.F = curcube.D;
        newcube.U = curcube.F;
        break;
    case 'F':
        newcube.F = curcube.F;
        newcube.B = curcube.B;
        newcube.U = curcube.L;
        newcube.L = curcube.D;
        newcube.D = curcube.R;
        newcube.R = curcube.U;
        break;
    case 'B':
        newcube.F = curcube.F;
        newcube.B = curcube.B;
        newcube.L = curcube.U;
        newcube.D = curcube.L;
        newcube.R = curcube.D;
        newcube.U = curcube.R;
        break;
    }
    return newcube;
}

void physics() {
    static float lastTime = glfwGetTime();
    float currentTime = glfwGetTime();
    if (currentRotate=="") {
        lastTime = currentTime;
        return;
    }
    float asp = freeSpeed;
    if (flag == 1) asp = genSpeed;
    else if (flag == 2) asp = solveSpeed;
    asp *= currentTime - lastTime;
    lastTime = currentTime;
    static float sp = 0.0f;
    if (abs(rotAngle)>((currentRotate[1]=='2')?90.0f:45.0f))
        sp -= asp;
    else sp += asp;
    if (currentRotate[1]=='\'') rotAngle -= sp;
    else rotAngle += sp;
    if (sp<=0.0f) {
        int count = 1;
        if (currentRotate[1]=='2') count = 2;
        else if (currentRotate[1] == '\'') count = 3;
        for (int i = 0;i<count;++i) {
            cube newcubes[9];
            switch (currentRotate[0]) {
            case 'U': {
                const int dU[] = {6, 3, 0, 7, 4, 1, 8, 5, 2};
                for (int i = 0;i<9;++i) newcubes[i] = rotateCube(cubes[dU[i]], currentRotate[0]);
                for (int i = 0;i<9;++i) cubes[dGU[i]] = newcubes[i];
                break;}
            case 'D':{
                const int dD[] = {20, 23, 26, 19, 22, 25, 18, 21, 24};
                for (int i = 0;i<9;++i) newcubes[i] = rotateCube(cubes[dD[i]], currentRotate[0]);
                for (int i = 0;i<9;++i) cubes[dGD[i]] = newcubes[i];
                break;}
            case 'L':{
                const int dL[] = {18, 9, 0, 21, 12, 3, 24, 15, 6};
                for (int i = 0;i<9;++i) newcubes[i] = rotateCube(cubes[dL[i]], currentRotate[0]);
                for (int i = 0;i<9;++i) cubes[dGL[i]] = newcubes[i];
                break;}
            case 'R':{
                const int dR[] = {8, 17, 26, 5, 14, 23, 2, 11, 20};
                for (int i = 0;i<9;++i) newcubes[i] = rotateCube(cubes[dR[i]], currentRotate[0]);
                for (int i = 0;i<9;++i) cubes[dGR[i]] = newcubes[i];
                break;}
            case 'F':{
                const int dF[] = {24, 15, 6, 25, 16, 7, 26, 17, 8};
                for (int i = 0;i<9;++i) newcubes[i] = rotateCube(cubes[dF[i]], currentRotate[0]);
                for (int i = 0;i<9;++i) cubes[dGF[i]] = newcubes[i];
                break;}
            case 'B':{
                const int dB[] = {2, 11, 20, 1, 10, 19, 0, 9, 18};
                for (int i = 0;i<9;++i) newcubes[i] = rotateCube(cubes[dB[i]], currentRotate[0]);
                for (int i = 0;i<9;++i) cubes[dGB[i]] = newcubes[i];
                break;}
            }
        }
        sp = 0.0f;
        rotAngle = 0.0f;
        if (rotations.empty()) {
            currentRotate = "";
            flag = 0;
        }
        else {
            currentRotate = rotations.front();
            rotations.pop();
        }
    }
}

void solve() {
    string scube, dcube;
    for (int i : gDSU) scube += cubes[i].U;
    for (int i : gDSR) scube += cubes[i].R;
    for (int i : gDSF) scube += cubes[i].F;
    for (int i : gDSD) scube += cubes[i].D;
    for (int i : gDSL) scube += cubes[i].L;
    for (int i : gDSB) scube += cubes[i].B;
    for (int i = 0;i<scube.size();++i) {
        if (scube[i] == 'B') dcube.push_back('F');
        else if (scube[i] == 'G') dcube.push_back('B');
        else if (scube[i] == 'O') dcube.push_back('L');
        else if (scube[i] == 'R') dcube.push_back('R');
        else if (scube[i] == 'Y') dcube.push_back('U');
        else dcube.push_back('D');
    }
    char patternized[64];
    char *sol = solution((char*)dcube.c_str(), 24, 1000, 0, "cache");
    if (sol == NULL) {
        cout<<"Unsolvable cube!"<<endl;
        return;
    }
    string ssol = sol;
    int last = 0;
    for (int i = 0;i<ssol.size();++i) {
        if (ssol[i] == ' ') {
            string tmp = "  "; tmp[0] = ssol[last];
            if (last+1!=i) tmp[1] = ssol[last+1];
            last = i+1;
            rotations.push(tmp);
        }
    }
    if (last!=ssol.size()) {
        string tmp = "  "; tmp[0] = ssol[last];
        if (last+1!=ssol.size()) tmp[1] = ssol[last+1];
        rotations.push(tmp);
    }
    if (currentRotate == "") {
        currentRotate = rotations.front();
        rotations.pop();
    }
}

void loadFromFile() {
    FILE *file = fopen("cache/shared", "r");
    unsigned char newcube[54], dcube[54];
    if (!file) {
        cout<<"File cache/shared could not open"<<endl;
        return;
    }
    if (fread(newcube,1,54,file) != 54) {
        cout<<"File cache/shared is corrupted"<<endl;
        return;
    }
    flag = 0;
    while (!rotations.empty()) {rotations.pop();}
    currentRotate = "";
    for (int i = 0;i<54;++i) {
        if (newcube[i] == 'U') dcube[i] = 'Y';
        else if (newcube[i] == 'D') dcube[i] = 'W';
        else if (newcube[i] == 'L') dcube[i] = 'O';
        else if (newcube[i] == 'R') dcube[i] = 'R';
        else if (newcube[i] == 'F') dcube[i] = 'B';
        else if (newcube[i] == 'B') dcube[i] = 'G';
    }
    int cnt = 0;
    for (int i: gDSU) {
        cubes[i].U = dcube[cnt];
        cnt++;
    }
    for (int i: gDSR) {
        cubes[i].R = dcube[cnt];
        cnt++;
    }
    for (int i: gDSF) {
        cubes[i].F = dcube[cnt];
        cnt++;
    }
    for (int i: gDSD) {
        cubes[i].D = dcube[cnt];
        cnt++;
    }
    for (int i: gDSL) {
        cubes[i].L = dcube[cnt];
        cnt++;
    }
    for (int i: gDSB) {
        cubes[i].B = dcube[cnt];
        cnt++;
    }
}

void render() {
    GLuint programID = LoadShaders("vertshader.vsh", "fragshader.fsh");
    GLuint matrixID = glGetUniformLocation(programID, "MVP");
    GLuint Texture = loadBMP("texture.bmp");
    GLuint TextureID = glGetUniformLocation(programID, "textureSampler");

    projection = perspective(radians(fov), width*1.0f/height, 0.1f, maxDistanse);
    model = mat4(1.0f);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    GLfloat vertex_buffer_data[324*9], uv_buffer_data[324*6];
    createVertices(vertex_buffer_data, uv_buffer_data);
    GLuint vertexbuffer;
    GLuint uvbuffer;
    glGenBuffers(1, &vertexbuffer);
    glGenBuffers(1, &uvbuffer);

    bool screenshot = true;
    float lastPressKey = -1e9f;
    float lastFPStime = glfwGetTime();
    float lastTimeFrame = glfwGetTime();
    int frames = 0;
    do {
        computeMatrices();
        physics();
        createVertices(vertex_buffer_data, uv_buffer_data);
        rotateVertices(vertex_buffer_data, uv_buffer_data);

        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_DYNAMIC_DRAW);

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glUseProgram(programID);
        glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glUniform1i(TextureID, 0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
        glDrawArrays(GL_TRIANGLES, 0, 324*3);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        if (screenshot) {
            glReadPixels(0,0,width,height,GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, argb_data);
            for (int y = 0;y<height;++y) {
                for (int x = 0;x<width;++x) {
                    int i1 = (y*width+x)*3;
                    int i2 = ((height-y-1)*width+x)*4;
                    rgb_data[i1] = argb_data[i2+3];
                    rgb_data[i1+1] = argb_data[i2+2];
                    rgb_data[i1+2] = argb_data[i2+1];
                }
            }
            saveImage("render.png", rgb_data);
            screenshot = false;
        }
        frames++;
        if ((glfwGetTime() - lastFPStime) > 1.0f) {
            //cout<<frames<<" fps"<<endl;
            frames = 0;
            lastFPStime += 1.0f;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
        float currentTime = glfwGetTime();
        usleep((int)1000000*std::max(0.0f, 1.0f/(fps + 0.5f) - (currentTime - lastTimeFrame)));
        lastTimeFrame = glfwGetTime();
        if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS && (glfwGetTime()-lastPressKey) > timePressKey) {
            screenshot = true;
            lastPressKey = glfwGetTime();
        }
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && (glfwGetTime()-lastPressKey) > timePressKey) {
            flag = 1;
            while (!rotations.empty()) {rotations.pop();}
            generateRotations(countGenRotations);
            lastPressKey = glfwGetTime();
        }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && (glfwGetTime()-lastPressKey) > timePressKey) {
            flag = 2;
            while (!rotations.empty()) {rotations.pop();}
            solve();
            lastPressKey = glfwGetTime();
        }
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && (glfwGetTime()-lastPressKey) > timePressKey) {
            loadFromFile();
            lastPressKey = glfwGetTime();
        }
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && (glfwGetTime()-lastPressKey) > timePressKey && !flag) {
            rotations.push("L2");
            if (currentRotate == "") {currentRotate = rotations.front(); rotations.pop();}
            lastPressKey = glfwGetTime();
        }
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && (glfwGetTime()-lastPressKey) > timePressKey && !flag) {
            rotations.push("R ");
            if (currentRotate == "") {currentRotate = rotations.front(); rotations.pop();}
            lastPressKey = glfwGetTime();
        }
        if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS && (glfwGetTime()-lastPressKey) > timePressKey && !flag) {
            rotations.push("U ");
            if (currentRotate == "") {currentRotate = rotations.front(); rotations.pop();}
            lastPressKey = glfwGetTime();
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && (glfwGetTime()-lastPressKey) > timePressKey && !flag) {
            rotations.push("D ");
            if (currentRotate == "") {currentRotate = rotations.front(); rotations.pop();}
            lastPressKey = glfwGetTime();
        }
        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && (glfwGetTime()-lastPressKey) > timePressKey && !flag) {
            rotations.push("B ");
            if (currentRotate == "") {currentRotate = rotations.front(); rotations.pop();}
            lastPressKey = glfwGetTime();
        }
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && (glfwGetTime()-lastPressKey) > timePressKey && !flag) {
            rotations.push("F ");
            if (currentRotate == "") {currentRotate = rotations.front(); rotations.pop();}
            lastPressKey = glfwGetTime();
        }
    }
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window)==0);
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteProgram(programID);
    glDeleteTextures(1, &Texture);
    glDeleteVertexArrays(1, &VertexArrayID);
    glfwTerminate();
}

int main() {
    if (init()) return 0;
    fillCubes();
    /*for (int i = 0;i<1260;++i) {
        rotations.push("R");
        rotations.push("U2");
        rotations.push("D'");
        rotations.push("B");
        rotations.push("D'");
    }
    if (currentRotate == "") {currentRotate = rotations.front(); rotations.pop();}
    */render();
    return 0;
}
