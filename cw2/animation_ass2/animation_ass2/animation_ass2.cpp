// animation_ass2.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <windows.h>
#include "gl\glut.h"
#include "ModelObj.h"
#include <string>
#include "RigidBody.h"

using namespace std;



#pragma warning(suppress : 4996)


// size of the window
static int WIDTH, HEIGHT;

// parameters for the camera
static float cam_rotateZ = 0.0f;
static float cam_rotateY = 0.0f;
static float cam_rotateX = -25.0f;
static float cam_distanceZ = 30.0f;
static float cam_distanceY = -1.0f;

// parameter for mouse events
static bool isLeftClicking = false;
static bool isRightClicking = false;
static bool isMiddleClicking = false;
int mouse_posX, mouse_posY;

// record outputfile path
string newSpherePath;
string newClothPath;
string newFloorPath;
ofstream outfile;

// models going to use in this program
ModelObj floorObj;
ModelObj sphere;
ModelObj cloth;
Vertex certre_of_sphere = Vertex(0.f,1.f,0.f);

float GRAVITY_G = -10.f;         //  unit: m/s^2
float CLOTH_MASS = 0.01f;       //  unit: kg


vector<Vertex> velocity_last_frame;
vector<Vertex> spring_force_vec;
vector<Spring> spring_vec;

float play_time = 0;
bool isPlaying  = true;
bool showBall   = true;
bool showFloor  = true;
bool isHanging  = false;
bool isSpinning = false;
bool isWinding  = false;
bool isDamped   = true;


// only for the hanging part
int leftCorner = -1;
int rightCorner = -1;

//=================================================================






void resetObj()
{
    // pause
    isPlaying = false;
    // reset timer
    play_time = 0;

    // clear model
    sphere   = ModelObj();
    cloth    = ModelObj();
    floorObj = ModelObj();

    // reading models
    string spherePath = "../sphere.obj";
    sphere.readObjFile(spherePath);

    string clothPath = "../cloth.obj";
    cloth.readObjFile(clothPath);

    string floorPath = "../floor.obj";
    floorObj.readObjFile(floorPath);

    newSpherePath = spherePath.substr(0, spherePath.size() - 4);
    newSpherePath += "_new.obj";

    newClothPath = clothPath.substr(0, spherePath.size() - 4);
    newClothPath += "_new.obj";

    newFloorPath = floorPath.substr(0, floorPath.size() - 4);
    newFloorPath += "_new.obj";

    // reset vectors
    velocity_last_frame.resize(cloth.vertices.size(), Vertex(0, 0, 0));
    spring_vec = RigidBody::generateSpringVec(cloth);
    spring_force_vec.resize(cloth.vertices.size(), Vertex(0, 0, 0));

    // play
    isPlaying = true;
}


void reshapeWindow(int width, int height)
{
    //set the size of canvas
    glViewport(0, 0, width, height);
    //coordinate transformation
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    //calculate aspect ratio and pass to perspective func
    double aspectratio = width / height;
    gluPerspective(50, aspectratio, 0.1, 200);

    //update height and width
    HEIGHT = height;
    WIDTH = width;
}

void setBackground(void)
{
    // information for the light source
    float specular[4] = { 0.9f, 0.9f, 0.9f, 1.0f };
    float diffuse[4] = { 0.7f, 0.7f, 0.7f, 1.0f };
    float ambient[4] = { 0.3f, 0.3f, 0.3f, 1.0f };
    float lightPos[4] = { 3.f, 6.f, 3.f, 1.f };

    // set the light
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    // enable light
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);

    // enable cull face test
    //glCullFace(GL_BACK);
    //glEnable(GL_CULL_FACE);

    // set the background colour
    glClearColor(0.3f, 0.4f, 0.5f, 1.0f);

}

void update_spring_force()
{
    // reset value evert iteration
    for (auto &force : spring_force_vec)
    {
        force = Vertex(0, 0, 0);
    }

    // calculate force on both end of each spring
    for (auto spring : spring_vec)
    {
        spring_force_vec[spring.leftID]  = spring_force_vec[spring.leftID] + spring.calculateLeftForce(cloth);
        spring_force_vec[spring.rightID] = spring_force_vec[spring.rightID] + spring.calculateRightForce(cloth);
    }
}

void updateModel(float delta_t)
{
    //update spring force for each vertex
    update_spring_force();
   
  
    // for each vertex, calculate position in next frame
    for (int i = 0; i < cloth.vertices.size(); i++)
    {
        //two corneo is fixed
        if (isHanging)
        {
            // don't update the position of the two corners
            if (i == cloth.leftCorner || i == cloth.rightCorner)
            {
                velocity_last_frame[i] = Vertex(0, 0, 0);
                continue;
            }
        }

        //gravity and spring force
        Vertex gravity_force = Vertex(0, GRAVITY_G * CLOTH_MASS, 0);
        Vertex total_force = gravity_force + spring_force_vec[i];

        // switches for the questions
        if (isWinding)
        {
            total_force = total_force + Vertex(0, 0, 0.05f);
        }
        if (isDamped)
        {
            total_force = total_force + velocity_last_frame[i] * -0.02f;
        }
        if (isSpinning)
        {
            if ((cloth.vertices[i] - certre_of_sphere).modulus() <= 1.25f)
            {
                Vertex dir = (cloth.vertices[i] - certre_of_sphere).normalize();
                Vertex friction = Vertex(0,1,0).cross((dir * 1.2f + certre_of_sphere)) * 2;
                total_force = total_force + friction;

            }
        }

        // calculate position in next frame, using formula  
        //      1) a = F/m, 
        //      2) v1 = v0 + a * delta_t, 
        //      3) x1 = x0 + v * delta_t
        Vertex acceleration = total_force / CLOTH_MASS;
        velocity_last_frame[i] = velocity_last_frame[i] + acceleration * delta_t;
        cloth.vertices[i] = cloth.vertices[i] + velocity_last_frame[i] * delta_t;

        // collision with the ball
        if (showBall)
        {
            if ((cloth.vertices[i] - certre_of_sphere).modulus() <= 1.25f)
            {
                Vertex dir = (cloth.vertices[i] - certre_of_sphere).normalize();
                cloth.vertices[i] = (dir * 1.2f + certre_of_sphere);
                velocity_last_frame[i] = Vertex(0, 0, 0);
            }
        }

        // collision with the floor
        if (showFloor)
        {
            if (cloth.vertices[i].y <= 0.1f)
            {
                cloth.vertices[i].y = 0.1f;
                velocity_last_frame[i].y = 0.f;
            }
        }
        

      
    }

   
}

void idle()
{
    static DWORD  prev_time = 0;
    DWORD  curr_time = timeGetTime();
    if (isPlaying && (curr_time - prev_time) * 0.001f >= 0.033f) 
    {
        updateModel(0.01f);
        prev_time = curr_time;
        play_time += 0.01f;
    }

    glutPostRedisplay();
}



//render the cloth and the sphere
void display() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, cam_distanceY, -cam_distanceZ);
    glRotatef(-cam_rotateX, 1.0, 0.0, 0.0);
    glRotatef(-cam_rotateY, 0.0, 1.0, 0.0);

    //set light
    float lightPos[4] = { 5.f, 6.f, 10.f, 1.f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);


    float scale = 1.0;


    //render sphere
    if (showBall && isPlaying)
    {
        for (auto& face : sphere.faces)
        {
            // draw wireframe
            glBegin(GL_TRIANGLES);


            glColor3f(1.f, 0.7f, 0.8f);
            Vertex v1 = sphere.getVert(face.vertices[0]);
            float x1 = v1.x * scale;
            float y1 = v1.y * scale;
            float z1 = v1.z * scale;

            Vertex n1 = sphere.getNorm(face.vertices[0]);
            glNormal3f(n1.x, n1.y, n1.z);
            glVertex3f(x1, y1, z1);

            Vertex v2 = sphere.getVert(face.vertices[1]);
            float x2 = v2.x * scale;
            float y2 = v2.y * scale;
            float z2 = v2.z * scale;

            Vertex n2 = sphere.getNorm(face.vertices[1]);
            glNormal3f(n2.x, n2.y, n2.z);
            glVertex3f(x2, y2, z2);


            Vertex v3 = sphere.getVert(face.vertices[2]);
            float x3 = v3.x * scale;
            float y3 = v3.y * scale;
            float z3 = v3.z * scale;

            Vertex n3 = sphere.getNorm(face.vertices[2]);
            glNormal3f(n3.x, n3.y, n3.z);
            glVertex3f(x3, y3, z3);


            glEnd();

        }
    }
    




    // render cloth
    if (isPlaying) 
    {
        for (auto& face : cloth.faces)
        {
            // draw wireframe
            glBegin(GL_TRIANGLES);

            glColor3f(0.5f, 1.f, 1.f);
            Vertex v1 = cloth.getVert(face.vertices[0]);
            float x1 = v1.x * scale;
            float y1 = v1.y * scale;
            float z1 = v1.z * scale;

            Vertex n1 = cloth.getNorm(face.vertices[0]);
            glNormal3f(n1.x, n1.y, n1.z);
            glVertex3f(x1, y1, z1);


            Vertex v2 = cloth.getVert(face.vertices[1]);
            float x2 = v2.x * scale;
            float y2 = v2.y * scale;
            float z2 = v2.z * scale;

            Vertex n2 = cloth.getNorm(face.vertices[1]);
            glNormal3f(n2.x, n2.y, n2.z);
            glVertex3f(x2, y2, z2);


            Vertex v3 = cloth.getVert(face.vertices[2]);
            float x3 = v3.x * scale;
            float y3 = v3.y * scale;
            float z3 = v3.z * scale;

            Vertex n3 = cloth.getNorm(face.vertices[2]);
            glNormal3f(n3.x, n3.y, n3.z);
            glVertex3f(x3, y3, z3);


            glEnd();

        }
    }




    //render floor
    if (showFloor && isPlaying)
    {
        for (auto& face : floorObj.faces)
        {
            // draw wireframe
            glBegin(GL_TRIANGLES);


            glColor3f(0.3f, 0.3f, 0.3f);
            Vertex v1 = floorObj.getVert(face.vertices[0]);
            float x1 = v1.x * scale;
            float y1 = v1.y * scale;
            float z1 = v1.z * scale;

            Vertex n1 = floorObj.getNorm(face.vertices[0]);
            glNormal3f(n1.x, n1.y, n1.z);
            glVertex3f(x1, y1, z1);


            Vertex v2 = floorObj.getVert(face.vertices[1]);
            float x2 = v2.x * scale;
            float y2 = v2.y * scale;
            float z2 = v2.z * scale;

            Vertex n2 = floorObj.getNorm(face.vertices[1]);
            glNormal3f(n2.x, n2.y, n2.z);
            glVertex3f(x2, y2, z2);


            Vertex v3 = floorObj.getVert(face.vertices[2]);
            float x3 = v3.x * scale;
            float y3 = v3.y * scale;
            float z3 = v3.z * scale;

            Vertex n3 = floorObj.getNorm(face.vertices[2]);
            glNormal3f(n3.x, n3.y, n3.z);
            glVertex3f(x3, y3, z3);


            glEnd();

        }
    }
    



    glPopMatrix();
    glutSwapBuffers();

}

void mouseEvents(int button, int state, int mouseX, int mouseY)
{
    //detect which button is clicked
    switch (button)
    {
    case (GLUT_LEFT_BUTTON):
        isLeftClicking = (state == GLUT_DOWN) ? 1 : 0;
        break;

    case (GLUT_RIGHT_BUTTON):
        isRightClicking = (state == GLUT_DOWN) ? 1 : 0;
        break;

    case (GLUT_MIDDLE_BUTTON):
        isMiddleClicking = (state == GLUT_DOWN) ? 1 : 0;
        break;
    }

    glutPostRedisplay();

    //record current cursor position
    mouse_posX = mouseX;
    mouse_posY = mouseY;

}

void moveCamera(int mouseX, int mouseY)
{
    // index, to be multiplied by the raw mouse data
    float index = 0.005f;

    //left-click to zoom
    if (isLeftClicking)
    {
        //zoom
        cam_distanceZ += index * (mouseY - mouse_posY);

        //add boundary
        if (cam_distanceZ < 5.0f)
        {
            cam_distanceZ = 5.0f;
        }
        if (cam_distanceZ > 100.0f)
        {
            cam_distanceZ = 100.0f;
        }
    }

    if (isRightClicking)
    {
        //rotate
        cam_rotateX -= index * (mouseY - mouse_posY);
        cam_rotateY -= index * (mouseX - mouse_posX);

        //add boundry
        if (cam_rotateX < -90.0f)
            cam_rotateX = -90.0f;
        else if (cam_rotateX > 90.0f)
            cam_rotateX = 90.0f;

        if (cam_rotateY < 0.0f)
            cam_rotateY += 360.0f;
        else if (cam_rotateY > 360.0f)
            cam_rotateY -= 360.0f;
    }


    glutPostRedisplay();
}

void keyboardEvents(unsigned char button, int mouseX, int mouseY)
{
    switch (button)
    {
        // pause / continue playing
        case ('p'):
            isPlaying = !isPlaying;
            break;

        // write obj
        case ('w'):

            isPlaying = false;

            outfile.open(newSpherePath, ios::out | ios::trunc);
            sphere.writeObj(outfile);
            outfile.close();

            outfile.open(newClothPath, ios::out | ios::trunc);
            cloth.writeObj(outfile);
            outfile.close();

            outfile.open(newFloorPath, ios::out | ios::trunc);
            floorObj.writeObj(outfile);
            outfile.close();

            isPlaying = true;

            break;

        // reset models
        case ('r'):
            resetObj();
            break;

        // switchs
        case('f'):
            showFloor = !showFloor;
            break;
        case('s'):
            showBall = !showBall;
            break;

        case('d'):
            isDamped = !isDamped;
            break;
        case('n'):
            isSpinning = !isSpinning;
            break;

        // task 3
        case('1'):
            showBall   = false;
            showFloor  = true;
            isHanging  = false;
            isSpinning = false;
            isWinding  = false;
            isDamped   = true;
            resetObj();
            break;

        // task 4
        case('2'):
            showBall   = true;
            showFloor  = true;
            isHanging  = false;
            isSpinning = false;
            isWinding  = false;
            isDamped   = true;
            resetObj();
            break;
        
        // task 5
        case('3'):
            showBall   = false;
            showFloor  = true;
            isHanging  = true;
            isSpinning = false;
            isWinding  = false;
            isDamped   = true;
            resetObj();
            break;

        // task 6
        case('4'):
            showBall   = true;
            showFloor  = true;
            isHanging  = false;
            isSpinning = true;
            isWinding  = false;
            isDamped   = true;
            resetObj();
            break;
            break;

        // task 7 with damping
        case('5'):
            showBall   = false;
            showFloor  = true;
            isHanging  = true;
            isSpinning = false;
            isWinding  = true;
            isDamped   = true;
            resetObj();
            break;

        // task 7 without damping
        case('6'):
            showBall   = false;
            showFloor  = true;
            isHanging  = true;
            isSpinning = false;
            isWinding  = true;
            isDamped   = false;
            resetObj();
            break;

            

        
        default:
            break;


    }


    glutPostRedisplay();
}



int main(int argc, char* argv[])
{
    resetObj();
    isPlaying = false;
    


    //ofstream outfile;
    //outfile.open(newPath, ios::out | ios::trunc);
    //sphere.writeObj(outfile);
    
   

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_STENCIL);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(960, 720);
    glutCreateWindow("obj_render");
    glutDisplayFunc(&display);
    glutReshapeFunc(&reshapeWindow);
    glutMouseFunc(&mouseEvents);
    glutKeyboardFunc(&keyboardEvents);
    glutMotionFunc(&moveCamera);
    glutIdleFunc(&idle);


    setBackground();
    glutMainLoop();


    return 1;

}

