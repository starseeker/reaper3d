#include "pmfactory.h"
#include "pmrealtimesys.h"
#include "main/types.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <GL/glut.h>

using namespace reaper::gfx::mesh;
using namespace reaper;

using std::cout;
using std::cin;

void create_mesh(PMFactory& pmf, int x_step, int y_step, float z_var, float param);
static void redraw(void);
static void idle(void);

bool overview;

PMRealTimeSys* rts_ptr = NULL;

void main(int argc, char** argv)
{
	if(false){
		char* file_name = "mesh.dat";
		
		SRMesh srmesh;
		PMFactory pmf(&srmesh);

		int d = 50;
		create_mesh(pmf, d, d, 10.0, 100.0 / (float) d);
		pmf.report(false);

		pmf.set_lower_bound(0);
		pmf.build();
		srmesh.save(file_name);
		return;
	}

		// initialize GLUT
		glutInit(&argc, argv);
		glutInitWindowSize(1000, 1000);
		glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH |  GLUT_ALPHA  );
		glutCreateWindow("foo");
		glutDisplayFunc(redraw);                                    //specifies our redraw function
		glutIdleFunc(idle);
		glDepthFunc(GL_LESS);
		glShadeModel(GL_SMOOTH);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);

		glMatrixMode(GL_PROJECTION);   //changes the current matrix to the projection matrix


		//sets up the projection matrix for a perspective transform
		gluPerspective(	 90,     //view angle
						 1.0,    //aspect ratio
						 10.0,   //near clip
						 2000.0); //far clip

		
		glMatrixMode(GL_MODELVIEW);   //changes the current matrix to the modelview matrix

		cout << "ready to go...\n";
		cin.get();

		glutMainLoop();               //the main loop of the GLUT framework


}

void testmesh(void)
{
	int tmp = 0;

	try {
		

		SRMesh srmesh;
		srmesh.load("mesh.dat");
		cout << "\nloaded mesh...\n";

		PMRealTimeSys rts(srmesh);
		rts.set_lower_bound(0);
		rts.set_max_stack_depth(90);
		rts_ptr = &rts;

		overview = false;

		cout << "refining...\n";
		float ang = 0.0;
		float cang = 1.0;
		float ox = 50.0;
		float oy = 50.0;
		float oz = 0.0;
		static int cnt = 0;
		for(;;){


			cnt++;
			float x = cos(ang) * 10;
			float y = sin(ang) * 10;
			float cx = cos(cang) * 30;
			float cy = sin(cang) * 20;
			float cz = 0.0;
			rts.set_cam(Point(ox + cx, oy +cy, oz + cz), Point(x, y, oz), 120);
			if(cnt % 10 == 0)rts.refine();
			ang += 0.003;
			cang += 0.001;

			if(!overview){
				glMatrixMode(GL_PROJECTION);   //changes the current matrix to the projection matrix
				glLoadIdentity();
				

				//sets up the projection matrix for a perspective transform
				gluPerspective(	 100,     //view angle
								 1.0,    //aspect ratio
								 0.1,   //near clip
								 2000.0); //far clip

				gluLookAt(ox + cx, oy + cy, oz + cz + 10.0, ox + cx + x, oy + cy + y, oz, 0, 0, 1);

				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
				glScalef(1.0, 1.0, 1.0);
			}
			redraw();
			//cin.get();
		}		
		
		

	} catch (SetTooSmall) {
		cout << "\nSetTooSmall\n";
	} catch (MalFormedInput) {
		cout << "\nMalFormedInput\n";
	} catch (ContractionOutOfDate) {
		cout << "\nContractionOutOfDate\n";
	} catch (MeshException e) {
		cout << "\n\nMeshException: " << e.error << "\n";
		redraw();
		cin.get();
		return;
	} catch (std::exception e) {
		cout << e.what() << "\n";
	}
}

static void idle(void)
{
	testmesh();

	return;
	exit(0);
}

static void redraw(void)                               //all drawing code goes here
{

		static Timer tim;

		static float z_rot = 0.0;
		static float tilt = 0.0;

		tilt += 0.03;
		if(tilt>360)tilt = 0.0;
		z_rot-= 0.05;
		if(z_rot>360)z_rot = 0.0;
	


	   // Set up the stationary light
		static float g_lightPos[4] = { 0, 0, -35, 1 };  // Position of light
	    glLightfv(GL_LIGHT0, GL_POSITION, g_lightPos);
		float light_ambient[]= {1,1,1,1};
		glLightfv(GL_LIGHT0,GL_AMBIENT, light_ambient);
		glLightfv(GL_LIGHT0,GL_SPECULAR, light_ambient);
		glLightfv(GL_LIGHT0,GL_DIFFUSE, light_ambient);
		float ambient[4] = {1.0, 1.0, 1.0, 1 }; 
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
//			GL_LIGHT_MODEL_AMBIENT	default (0.2,0.2,0.2,1.0)



		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  //clears the colour and depth buffers
		glEnable(GL_DEPTH_TEST);
		
		glPushMatrix();         //saves the current matrix on the top of the matrix stack

		if(overview){
			glTranslatef(0,0, -60);

			//glRotatef(z_rot, 0, 0, 1);
			glRotatef(10, cos(tilt), sin(tilt), 1);
			//glRotatef(tilt, 1, 0, 0);
			glTranslatef(-50, -50, 0);
		}

			
			//glRotatef(z_rot, cos(tilt), sin(tilt), 1);

		if(rts_ptr != NULL)(*rts_ptr).render();

		glPopMatrix();          //retrieves our saved matrix from the top of the matrix stack

		glutSwapBuffers();

		tim.stop();
		(*rts_ptr).render_cnt++;
		tim.start();

}	

float get_z(float max)
{
	//return 1.0;
	static float reso = 100000;
	return  ((float) ( rand() % ( (int) (max * reso) )) ) / (reso / max);
   //return 1.0;
/*
   static float f = 0.0;
   f += 0.1;
   return f;
  */  
   float r0 = (float) rand();
   float r1 = (float) rand();

   float r3 = r0 / r1;

   int i = (int) r3;
   int d = i - (i%( (int) max));

   r3 -= d;

   return r3;
}



void create_mesh(PMFactory& pmf, int x_step, int y_step, float z_var, float param)
{
	Point p;
	float z = 0.0;
	int x,y;

	for(y = 0; y < y_step; y++){
		for(x = 0; x < x_step; x++){
			
			p = Point(x * param, y * param, get_z(z_var));

			pmf.new_vertex(p);

		}
	}

	for(y = 0; y < y_step - 1; y++){
		for(x = 0; x < x_step - 1; x++){

			int p0 = y * x_step + x;
			int p1 = p0 + 1;
			int p2 = (y + 1) * x_step + x;
			int p3 = p2 + 1;

			pmf.new_face(p0, p1, p2);
			pmf.new_face(p1, p3, p2);
		}
	}

}
