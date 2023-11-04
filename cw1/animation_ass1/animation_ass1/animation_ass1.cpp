
//reference: the logic of the GUI was developed based on the inspiration of the code written by Oshita Laboratory, 
//  which can be found on: https://github.com/HW140701/LoadandDisplayBVH/blob/master/src/BVHPlayer.cpp
//  I added my own design but referenced the logic of that code


#include <iostream>
#include "BVH.h"
#include <gl/glut.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <windows.h>

#include <glm/glm.hpp>
#include <glm/vec3.hpp>



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

// BVH pointer
BVH * myBVHfile = nullptr;

// playing parameter
int frame = 0;
float play_time = 0.0f;
bool isPlaying = true;

BVH::Joint* IK_joint;



void IKHandle()
{
    vector<float> theta;
    for (auto joint : myBVHfile->joints)
    {
        if (!joint->parent) 
        {
            for (int i = 3; i < 6; i++)
            {
                int index = joint->channels[i]->index;
                theta.push_back(myBVHfile->GetMotion(frame, index));
            }
        }
        else
        {
            for (int i = 0; i < 3; i++)
            {
                int index = joint->channels[i]->index;
                theta.push_back(myBVHfile->GetMotion(frame, index));
            }
        }
    }
    //for (auto item : theta) cout << item << endl;
    //cout << theta.size() << endl;
    //cout << myBVHfile->GetNumJoint() << endl;
    return;
}


void draw(void)
{

    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_ACCUM_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, cam_distanceY, -cam_distanceZ);
    glRotatef(-cam_rotateX, 1.0, 0.0, 0.0);
    glRotatef(-cam_rotateY, 0.0, 1.0, 0.0);

    //set light
    float lightPos[4] = { 5.f, 0.f, 10.f, 1.f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    //start to draw the floor
    glBegin(GL_QUADS);
    glColor3f(0.95, 0.95, 0.95);
    glVertex3f(-5.0, 0.0, -5.0);  //top: left-bot
    glVertex3f(-5.0, 0.0, 5.0);   //top: left-top
    glVertex3f(5.0, 0.0, 5.0);    //top: right-top
    glVertex3f(5.0, 0.0, -5.0);   //top: right-bot

    glVertex3f(5.0, -0.5, -5.0);   //bot: right-bot
    glVertex3f(5.0, -0.5, 5.0);    //bot: right-top
    glVertex3f(-5.0, -0.5, 5.0);   //bot: left-top
    glVertex3f(-5.0, -0.5, -5.0);  //bot: left-bot

    glColor3f(0.7, 0.7, 0.7);
    glVertex3f(-5.0, -0.5, 5.0);  //left: left-bot
    glVertex3f(-5.0, 0.0, 5.0);   //left: left-top
    glVertex3f(-5.0, 0.0, -5.0);    //left: right-top
    glVertex3f(-5.0, -0.5, -5.0);   //left: right-bot

    glVertex3f(-5.0, -0.5, -5.0);  //front: left-bot
    glVertex3f(-5.0, 0.0, -5.0);   //front: left-top
    glVertex3f(5.0, 0.0, -5.0);    //front: right-top
    glVertex3f(5.0, -0.5, -5.0);   //front: right-bot

    glVertex3f(5.0, -0.5, 5.0);  //back: left-bot
    glVertex3f(5.0, 0.0, 5.0);   //back: left-top
    glVertex3f(-5.0, 0.0, 5.0);    //back: right-top
    glVertex3f(-5.0, -0.5, 5.0);   //back: right-bot

    glVertex3f(5.0, -0.5, -5.0);  //right: left-bot
    glVertex3f(5.0, 0.0, -5.0);   //right: left-top
    glVertex3f(5.0, 0.0, 5.0);    //right: right-top
    glVertex3f(5.0, -0.5, 5.0);   //right: right-bot

    glEnd();

    //start to draw model

    glColor3f(0.5f, 0.5f, 0.0f);
    if (myBVHfile != nullptr)
    {
        myBVHfile->RenderFigure(frame, 0.15f);
    }


    glutSwapBuffers();
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
        case ('t'):
            IKHandle();
            break;
        //= pressed, to move to the next frame
        case ('='):
            play_time += myBVHfile->GetInterval();
            frame++;
            frame = frame % myBVHfile->GetNumFrame();
            break;
        //- pressed, to move to the previous frame
        case ('-'):
            play_time -= myBVHfile->GetInterval();
            frame--;
            frame = frame % myBVHfile->GetNumFrame();
            break;
        //P pressed, to pause the animation
        case ('p'):
            isPlaying = !isPlaying;
            break;
        //W pressed, to write a BVH file
        case ('w'):
            myBVHfile->writeBVH(myBVHfile);
            break;
        //L pressed, to load BVH file
        case ('l'):

            char file_name[1024] = "";                      // create a char[] to store file name 
            OPENFILENAME ofn;                               // the file needed to be opened
            memset(&ofn, 0, sizeof(OPENFILENAME));          // allocate memory
            ofn.lStructSize = sizeof(OPENFILENAME);          
            ofn.hwndOwner = NULL;                           // no parent window
            ofn.lpstrFilter = "BVH Motion Data (*.bvh)\0*.bvh\0All (*.*)\0*.*\0";       // set the format
            ofn.nFilterIndex = 1;
            ofn.lpstrFile = file_name;
            ofn.nMaxFile = 1024;
            ofn.lpstrTitle = "Please choose a BVH file";            // interface
            ofn.lpstrDefExt = "bvh";                                // set the suffix
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
            BOOL  got_file_name = GetOpenFileName(&ofn);        //get the file

            if (got_file_name)
            {
                if (myBVHfile != nullptr)
                {
                    // myBVHfile has already existed
                    delete myBVHfile;
                }

                //pass the file to the pointer
                myBVHfile = new BVH(file_name);

                //check if successfully loaded
                if (myBVHfile->IsLoadSuccess() == false)
                {
                    //fail to load the file
                    cout << "fail to load this file: " << file_name << endl;
                    delete myBVHfile;
                    myBVHfile = NULL;
                }
            }

            frame = 0;
            play_time = 0.0f;

            break;

        
    }


    glutPostRedisplay();
}

void setBackground(void)
{
    // information for the light source
    float specular[4] = { 0.9f, 0.9f, 0.9f, 1.0f };
    float diffuse[4] = { 0.7f, 0.7f, 0.7f, 1.0f };
    float ambient[4] = { 0.3f, 0.3f, 0.3f, 1.0f };
    float lightPos[4] = { 5.f, 0.f, 10.f, 1.f };

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
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    // set the background colour
    glClearColor(0.3f, 0.4f, 0.5f, 1.0f);

}




void  update(void)
{
    if (isPlaying)
    {
        static DWORD  prev_time = 0;
        DWORD  curr_time = timeGetTime();
        if ((curr_time - prev_time) * 0.001f > 0.03f) {
            play_time += 0.03f;
        }
        else {
            play_time += (curr_time - prev_time) * 0.001f;
        }
        prev_time = curr_time;
        
        if (myBVHfile)
        {
            frame = play_time / myBVHfile->GetInterval();
            frame %= myBVHfile->GetNumFrame();
        }
        else
            frame = 0;

        glutPostRedisplay();
    }
}




int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_STENCIL);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(960, 720);
    glutCreateWindow("my BVH player");

    glutDisplayFunc(&draw);
    glutReshapeFunc(&reshapeWindow);
    glutMouseFunc(&mouseEvents);
    glutMotionFunc(&moveCamera);
    glutKeyboardFunc(&keyboardEvents);
    glutIdleFunc(&update);

    setBackground();
    glutMainLoop();
}


