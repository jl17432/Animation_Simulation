// animation_ass2.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <windows.h>
#include "gl\glut.h"
#include "ModelObj.h"
#include "Particle.h"
#include <set>
#include <unordered_map>

using namespace std;



#pragma warning(suppress : 4996)


// size of the window
static int WIDTH, HEIGHT;

// parameters for the camera
static float cam_rotateZ = 0.0f;
static float cam_rotateY = 0.0f;
static float cam_rotateX = 0.0f;
static float cam_distanceZ = 70.0f;
static float cam_distanceY = -30.0f;

// parameter for mouse events
static bool isLeftClicking = false;
static bool isRightClicking = false;
static bool isMiddleClicking = false;
int mouse_posX, mouse_posY;




float play_time = 0;
bool isPlaying  = true;
bool showTank = true;

ModelObj myDisk;        //disk with radius = 0.2m
vector<Particle> particleVec;
vector<vector<set<int>>> grid;


//set gravity costants
float particlemass;
float GRAVITY_G;         //  unit: m/s^2
Vertex gravity = Vertex(0.f, particlemass * GRAVITY_G, 0.f);


float radius_of_particle;
float neighbour_ring_radius;
float gas_constant;
float rest_density;
float miu;


float left_boundary = -24.55f;
float right_boundary = 25.55f;
float bottom_boundary = 1.f;
float top_boundary = 65.f;
float boundary_force_index = -23.5f;



//kernels:
bool isPoly6 = true;
bool isSpiky = false;
bool isViscosity = false;

float poly6_kernel_constant;
float poly6_kernel_gradient_constant;
float spiky_kernel_constant;
float spiky_kernel_gradient_constant;
float viscosity_kernel_constant;
//float viscosity_kernel_gradient_constant;


//=================================================================

float poly6Kernel(float r, float h)
{
    float h2_minus_r2 = h * h - r * r;
    return  poly6_kernel_constant * h2_minus_r2 * h2_minus_r2 * h2_minus_r2;
}

float poly6Kernel_gradient(float r, float h)
{
    float h2_minus_r2 = h * h - r * r;
    return poly6_kernel_gradient_constant * r * h2_minus_r2 * h2_minus_r2;
}

float spikyKernel(float r, float h)
{
    float h_minus_r = h - r;
    return spiky_kernel_constant * h_minus_r * h_minus_r * h_minus_r;
}

float spikyKernel_gradient(float r, float h)
{
    float h_minus_r = h - r;
    return spiky_kernel_gradient_constant * r * h_minus_r * h_minus_r;
}

float viscosityKernel(float r, float h)
{
    return viscosity_kernel_constant * (h - r);

}

void updateParticles(float delta_t)
{


    for (int i = 0; i < particleVec.size(); i++)
    {
        particleVec[i].density = 0;
        particleVec[i].neighbours.clear();

        for (int j = 0; j < particleVec.size(); j++)
        {
            //if (i == 0) cout << j << endl;
            if (j == i) continue;
            Vertex direction = particleVec[i].position - particleVec[j].position;
            float distance = direction.modulus();
            if (distance < 0.f || distance > neighbour_ring_radius) continue;
            if (isPoly6)
            {
                particleVec[i].density += particlemass * poly6Kernel(distance, neighbour_ring_radius);
            }
            else if (isSpiky)
            {
                particleVec[i].density += particlemass * spikyKernel(distance, neighbour_ring_radius);
            }
            
            particleVec[i].neighbours.push_back(j);
        }
      
    }


    for (int i = 0; i < particleVec.size(); i++)
    {
        if (particleVec[i].neighbours.size() > 0)
        {
            Vertex total_viscosity;
            for (auto& neighourID : particleVec[i].neighbours)
            {
                Vertex dir = particleVec[i].position - particleVec[neighourID].position;
                float distance = dir.modulus();
                //dir.normalize();

                // calculate pressure force
                float pi_plus_pj = gas_constant * (particleVec[i].density + particleVec[neighourID].density - 2 * rest_density);
                if (isPoly6)
                {
                    particleVec[i].force = particleVec[i].force - dir *(pi_plus_pj / 2 * particleVec[neighourID].density * poly6Kernel_gradient(distance, neighbour_ring_radius));
                }
                else if (isSpiky)
                {
                    particleVec[i].force = particleVec[i].force - dir * (pi_plus_pj / 2 * particleVec[neighourID].density * spikyKernel_gradient(distance, neighbour_ring_radius));
                }

                // calculate viscosity force
                if (isViscosity)
                {
                    if (particleVec[neighourID].density != 0)
                    {
                        Vertex viscosity = (particleVec[neighourID].velocity - particleVec[i].velocity) * particlemass / particleVec[neighourID].density * viscosityKernel(distance, neighbour_ring_radius);

                        total_viscosity = total_viscosity + viscosity;
                    }
                }
                
            }
            total_viscosity = total_viscosity * miu;
            particleVec[i].force = particleVec[i].force + total_viscosity;
        }
    }


    for (int i = 0; i < particleVec.size(); i++)
    {
        particleVec[i].force = particleVec[i].force + gravity;
        particleVec[i].updateVelocity(delta_t);
        particleVec[i].updatePosition(delta_t);
        particleVec[i].force = Vertex(0, 0, 0);

        if (particleVec[i].position.y - 1 <= bottom_boundary)
        {
            if (particleVec[i].velocity.y < 0)
            {
                particleVec[i].force.y += particleVec[i].velocity.y * boundary_force_index;
                particleVec[i].position.y = 0;
                particleVec[i].velocity.y *= -1;

            }
        }
        else if (particleVec[i].position.y + 3 >= top_boundary)
        {
            if (particleVec[i].velocity.y > 0)
            {
                particleVec[i].force.y += particleVec[i].velocity.y * boundary_force_index;
                particleVec[i].position.y = top_boundary;
            }
        }
        if (particleVec[i].position.x - 1 <= left_boundary)
        {
            if (particleVec[i].velocity.x < 0)
            {
                particleVec[i].force.x += particleVec[i].velocity.x * boundary_force_index;
                particleVec[i].position.x = left_boundary;
            }
        }
        else if (particleVec[i].position.x + 1 >= right_boundary)
        {
            if (particleVec[i].velocity.x > 0)
            {
                particleVec[i].force.x += particleVec[i].velocity.x * boundary_force_index;
                particleVec[i].position.x = right_boundary;
            }
        }


       

       
        
    }
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
    gluPerspective(50, aspectratio, 0.1, 300);

    //update height and width
    HEIGHT = height;
    WIDTH = width;
}

void setBackground(void)
{
    // information for the light source
    //float specular[4] = { 0.9f, 0.9f, 0.9f, 1.0f };
    //float diffuse[4] = { 0.7f, 0.7f, 0.7f, 1.0f };
    //float ambient[4] = { 0.3f, 0.3f, 0.3f, 1.0f };
    float specular[4] = { 0.f, 0.f, 0.f, 1.0f };
    float diffuse[4] = { 0.f, 0.f, 0.f, 1.0f };
    float ambient[4] = { 1.f, 1.f, 1.f, 1.0f };
    float lightPos[4] = { 0.f, 0.f, 10.f, 1.f };

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


    // set the background colour
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

}






void idle()
{
    static DWORD  prev_time = 0;
    DWORD  curr_time = timeGetTime();
    if (isPlaying && (curr_time - prev_time) * 0.001f >= 0.033f) 
    {
        updateParticles(0.07f);
        prev_time = curr_time;
        play_time += 0.07f;
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
    float lightPos[4] = { 0.f, 0.f, 10.f, 1.f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    if (showTank)
    {
        // draw water tank
        glBegin(GL_QUADS);
        glColor3f(0.7, 0.6, 0.5);

        // left
        glVertex3f(-24.0, 50.0, 0.0);  
        glVertex3f(-25.0, 50.0, 0.0);   
        glVertex3f(-25.0, -1.0, 0.0);   
        glVertex3f(-24.0, -1.0, 0.0);  

        // bottom
        glVertex3f(-25.0, -0.1, 0.0);
        glVertex3f(25.0, -0.1, 0.0);
        glVertex3f(25.0, -1.0, 0.0);
        glVertex3f(-25.0, -1.0, 0.0);

        // right
        glVertex3f(25.0, 50.0, 0.0);
        glVertex3f(26.0, 50.0, 0.0);
        glVertex3f(26.0, -1.0, 0.0);
        glVertex3f(25.0, -1.0, 0.0);
    }
   
    
    
    for (auto& particle : particleVec)
    {
        for (auto& face : myDisk.faces)
        {
            // draw particles
            glBegin(GL_TRIANGLES);


            glColor3f(0.2f, 0.5f, 0.8f);
            Vertex v1 = myDisk.getVert(face.vertices[0]);
            float x1 = v1.x + particle.position.x;
            float y1 = v1.y + particle.position.y;
            float z1 = v1.z + particle.position.z;

            Vertex n1 = myDisk.getNorm(face.vertices[0]);
            glNormal3f(n1.x, n1.y, n1.z);
            glVertex3f(x1, y1, z1);

            Vertex v2 = myDisk.getVert(face.vertices[1]);
            float x2 = v2.x + particle.position.x;
            float y2 = v2.y + particle.position.y;
            float z2 = v2.z + particle.position.z;

            Vertex n2 = myDisk.getNorm(face.vertices[1]);
            glNormal3f(n2.x, n2.y, n2.z);
            glVertex3f(x2, y2, z2);

            Vertex v3 = myDisk.getVert(face.vertices[2]);
            float x3 = v3.x + particle.position.x;
            float y3 = v3.y + particle.position.y;
            float z3 = v3.z + particle.position.z;

            Vertex n3 = myDisk.getNorm(face.vertices[2]);
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

void generateParticles()
{
    // generate particleVec
    particleVec.clear();
    Vertex position;
    for (float x = -12.f; x < 8.f; x += radius_of_particle)
    {
        for (float y = 25.f; y < 55.f; y += radius_of_particle)
        {
            position = Vertex(x, y, 0);
            particleVec.push_back(Particle(position));
        }
    }

    //generate grid

    grid.clear();
    grid.resize(50);
    for (auto& row : grid)
    {
        row.resize(80);
    }

    //push particles into the grid
    for (int i = 0; i < particleVec.size(); i++)
    {
        int col = particleVec[i].col_inGrid;
        int row = particleVec[i].row_inGrid;
        cout << col << " | " << row << endl;
        grid[col][row].insert(i);
    }
}

void keyboardEvents(unsigned char button, int mouseX, int mouseY)
{
    switch (button)
    {
        // pause / continue playing
        case ('p'):
            isPlaying = !isPlaying;
            break;

        case ('1'):
            isPlaying = false;
            isPoly6 = true;
            isSpiky = false;
            isViscosity = false;
            generateParticles();
            isPlaying = true;
            break;

        case ('2'):
            isPlaying = false;
            isPoly6 = false;
            isSpiky = true;
            isViscosity = false;
            generateParticles();
            isPlaying = true;
            break;

        case ('3'):
            isPlaying = false;
            isPoly6 = false;
            isSpiky = true;
            isViscosity = true;
            generateParticles();
            isPlaying = true;
            break;
       
            

        
        default:
            break;


    }


    glutPostRedisplay();
}


void setVar() 
{
    if (isPoly6)
    {
        particlemass = 1.f;
        GRAVITY_G = -6.f;
        gravity = Vertex(0.f, particlemass * GRAVITY_G, 0.f);
        radius_of_particle = 1.f;
        neighbour_ring_radius = 4.7f;
        gas_constant = 500.f;
        rest_density = 0.15f;
        miu = 4.5f;
    }

    else if (isSpiky)
    {
        particlemass = 1.f;
        GRAVITY_G = -6.f;
        gravity = Vertex(0.f, particlemass * GRAVITY_G, 0.f);
        radius_of_particle = 1.f;
        neighbour_ring_radius = 4.7f;
        if (!isViscosity) gas_constant = 50.f;
        else gas_constant = 100.f;
        rest_density = 1.3f;
        miu = 4.5f;
    }
}




int main(int argc, char* argv[])
{
   //read in disk.obj
    isPlaying = true;
    string diskPath = "../disk1.obj";
    myDisk.readObjFile(diskPath);
    setVar();

    //compute poly6 kernel constant:
    float h_9 = pow(neighbour_ring_radius, 9);
    poly6_kernel_constant = 315 / (64 * 3.14159 * h_9);
    poly6_kernel_gradient_constant = -945 / (32 * 3.14159 * h_9);

    // compute spiky kernel constant:
    spiky_kernel_constant = 15 / (3.14159 * pow(neighbour_ring_radius, 6));
    spiky_kernel_gradient_constant = -45 / (3.14159 * pow(neighbour_ring_radius, 6));

    // compute viscosity kernel constant:
    viscosity_kernel_constant = 45 / (3.14159 * pow(neighbour_ring_radius, 6));

    
    
    generateParticles();
    

    // render part

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_STENCIL);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(900, 900);
    glutCreateWindow("fluid_simulation");
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

