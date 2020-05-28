#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<windows.h>
#include<GL/glut.h>
#include <string>
#include<time.h>

//
double transletx,translety,transletz,carrotation,wheelrotate;
double carspeed;
//
double cameraAngle;
double move_X, move_Y, move_Z;
int canDrawGrid, canDrawAxis;
double speed=6,ang_speed=.001;
double cameraRadius, cameraHeight, cameraAngleDelta;
int num_texture  = -1;
GLuint lighting,bmw,grassimg,roadmain,sceenarioleft,sceenarioright,cylender;

double translateyalarm,ballty,bonus,point;
int balli,jump,jumpi;

/***************************** VECTOR structure **********************************/

struct V;

V operator+(V a,V b);
V operator*(V a,V b);
V operator*(V b,double a);
V operator*(double a,V b);

struct V{
	double x,y,z;

	V(){}
	V(double _x,double _y){x=_x;y=_y;z=0;}
	V(double _x,double _y,double _z){x=_x;y=_y;z=_z;}

	
	double	mag(){	return sqrt(x*x+y*y+z*z);	}
	
	void 	norm(){	double d = mag();x/=d;y/=d;	z/=d;}
	V 		unit(){	V ret = *this;	ret.norm(); return ret;}

	double	dot(V b){		return x*b.x + y*b.y + z*b.z;}
	V		cross(V b){		return V( y*b.z - z*b.y , z*b.x - x*b.z , x*b.y - y*b.x );}
	

	double	projL(V on){	on.norm();	return this->dot(on);}
	V		projV(V on){	on.norm();	return on * projL(on);}

	V rot(V axis, double angle){
		return this->rot(axis, cos(angle), sin(angle));
	}
	
	V rot(V axis, double ca, double sa){
		V rotatee = *this;
		axis.norm();
		V normal = (axis * rotatee).unit();
		V mid = (normal * axis).unit();
		double r = rotatee.projL(mid);
		V ret=r*mid*ca + r*normal*sa + rotatee.projV(axis);
		return ret.unit();
	}
};

V operator+(V a,V b){		return V(a.x+b.x, a.y+b.y, a.z+b.z);	}
V operator-(V a){			return V (-a.x, -a.y, -a.z);			}
V operator-(V a,V b){		return V(a.x-b.x, a.y-b.y, a.z-b.z);	}
V operator*(V a,V b){		return a.cross(b);						}
V operator*(double a,V b){	return V(a*b.x, a*b.y, a*b.z);			}
V operator*(V b,double a){	return V(a*b.x, a*b.y, a*b.z);			}
V operator/(V b,double a){	return V(b.x/a, b.y/a, b.z/a);			}




V loc,dir,perp;


V  _L(0,-150,20);
V  _D(0,1,0);
V  _P(0,0,1);

/***************************** Texture Functions *******************************/

int LoadBitmapImage(char *filename)
{
    int i, j=0;
    FILE *l_file;
    unsigned char *l_texture;

    BITMAPFILEHEADER fileheader;
    BITMAPINFOHEADER infoheader;
    RGBTRIPLE rgb;

    num_texture++;

    if( (l_file = fopen(filename, "rb"))==NULL) return (-1);

    fread(&fileheader, sizeof(fileheader), 1, l_file);

    fseek(l_file, sizeof(fileheader), SEEK_SET);
    fread(&infoheader, sizeof(infoheader), 1, l_file);

    l_texture = (byte *) malloc(infoheader.biWidth * infoheader.biHeight * 4);
    memset(l_texture, 0, infoheader.biWidth * infoheader.biHeight * 4);
	for (i=0; i < infoheader.biWidth*infoheader.biHeight; i++)
		{
				fread(&rgb, sizeof(rgb), 1, l_file);

				l_texture[j+0] = rgb.rgbtRed;
				l_texture[j+1] = rgb.rgbtGreen;
				l_texture[j+2] = rgb.rgbtBlue;
				l_texture[j+3] = 255;
				j += 4;
		}
    fclose(l_file);

    glBindTexture(GL_TEXTURE_2D, num_texture);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, infoheader.biWidth, infoheader.biHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, l_texture);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 4, infoheader.biWidth, infoheader.biHeight, GL_RGBA, GL_UNSIGNED_BYTE, l_texture);

    free(l_texture);

    return (num_texture);

}

void loadImage()
{
	lighting = LoadBitmapImage("images/light.bmp");
	bmw = LoadBitmapImage("images/bmw.bmp");
	grassimg = LoadBitmapImage("images/grass.bmp");
	roadmain = LoadBitmapImage("images/road.bmp");
	sceenarioleft = LoadBitmapImage("images/sceenario2.bmp");
	sceenarioright = LoadBitmapImage("images/sceenario.bmp");
		cylender=LoadBitmapImage("images/cyl.bmp");
	if(lighting != -1)
		printf("Load successful!!\n");
	else printf("Image loading Failed!!\n");
}

/****************************** GridLines and Axes ***********************************/
//////////////////////////////////////////////my code

void drawText(const char *text, int length, int x, int y){
 glMatrixMode(GL_PROJECTION);  
 double matrix[16]; 
 glGetDoublev(GL_PROJECTION_MATRIX, matrix); 
 glLoadIdentity(); 
 glOrtho(0, 800, 0, 600, -5, 5); 
 glMatrixMode(GL_MODELVIEW);  
 glLoadIdentity();  
 glPushMatrix();  
 glLoadIdentity();  
 glRasterPos2i(x, y);  
 
 for(int i=0; i<length; i++){
  glutBitmapCharacter(GLUT_BITMAP_9_BY_15, (int)text[i]);  
 }
 glPopMatrix();  
 glMatrixMode(GL_PROJECTION); 
 glLoadMatrixd(matrix);  
 glMatrixMode(GL_MODELVIEW);  
}




void wheelstand(int rotate){
	glPushMatrix();{
	glRotatef(rotate,0,1,0);
	glRotatef(wheelrotate,0,1,0);
		glScalef(0.5,0.5,2);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);
	}glPopMatrix();
}

void wheel(int tx,int ty,int tz,int r){

	glTranslatef(tx,ty,tz);
	glRotatef(r,1,0,0);
	
	glPushMatrix();{
	glPushMatrix();{
		glColor3f(0,0,0);
		glTranslatef(1,0.2,0);
		glScalef(1,1,1);
		glRotatef(90,1,0,0);
		glutSolidTorus(1.5,3,20,20);

	}glPopMatrix();


	glPushMatrix();{
		
		glTranslatef(1,-1,0.2);
		for (int i = 0; i < 6; i++)
		{
			if(i%2==0){
				glColor3f(1,1,1);
			}else{
			glColor3f(.8,.2,0);
			}
			wheelstand(i*60);
	
		}
	}glPopMatrix();

	}glPopMatrix();
}

void lookingglass(){
	glTranslatef(0,1.6,-4);
	glPushMatrix();{
	glColor3f(.2,.2,.2);
	glTranslatef(0,-2,11);
 
	glScalef(1,2,1);
	glTranslatef(0.5,0.5,0.5);
	glutSolidCube(1);
	}glPopMatrix();

	//glass
	glPushMatrix();{
			glColor3f(.2,.2,.2);
		glTranslatef(0,-5.8,10);
		glScalef(0.5,2,1.6);
		 
		glTranslatef(1,1,1);
		glutSolidSphere(1,20,20);
	}glPopMatrix();

	//glass
	glPushMatrix();{
			glColor3f(1,1,1);
		glTranslatef(0.8,-5,10.7);
		glScalef(0.2,1.2,1);
	 
		glTranslatef(1,1,1);
		glutSolidSphere(1,20,20);
	}glPopMatrix();

}

 void lookingglass2(){
	glTranslatef(0,20.5,-4);
	glPushMatrix();{
	glColor3f(.2,.2,.2);
	glTranslatef(0,-2,11);
 
	glScalef(1,2,1);
	glTranslatef(0.5,0.5,0.5);
	glutSolidCube(1);
	}glPopMatrix();

	//glass
	glPushMatrix();{
			glColor3f(.2,.2,.2);
		glTranslatef(0,0,10);
		glScalef(0.5,2,1.6);
		 
		glTranslatef(1,1,1);
		glutSolidSphere(1,20,20);
	}glPopMatrix();

	//glass
	glPushMatrix();{
			glColor3f(1,1,1);
		glTranslatef(0.8,0.8,10.8);
		glScalef(0.2,1.2,1);
	 
		glTranslatef(1,1,1);
		glutSolidSphere(1,20,20);
	}glPopMatrix();

}

 void light(int x){
 
	 glPushMatrix();{
	 
		glTranslatef(32.6,2.4+x,5.5);
		glScalef(1.5,2.5,1.5);
		glTranslatef(0.5,0.5,0.5);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,lighting);
	glColor3f(1,1,1);
	GLUquadricObj *sphere=gluNewQuadric();
	gluQuadricTexture(sphere,GL_TRUE);
gluSphere(sphere,1,20,20);
	glDisable(GL_TEXTURE_2D);




	}glPopMatrix();
 
 
 }

void car(double tx,double ty,double tz,double r){

	glTranslatef(tx,ty,tz);
   	glRotatef(-90+r,0,0,1);
	glScalef(1.4,1.2,1);
	glTranslatef(0,0,6);
	//middle under
	glPushMatrix();{
		glColor3f(.3,.3,.3);
		glScalef(30,20,10);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);

	}glPopMatrix();

	//middle upper
	glPushMatrix();{
		glColor3f(.4,0,.2);
		glTranslatef(7,0.3,5);
		glScalef(16,19.4,10);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);

	}glPopMatrix();


		//middle upper 1
	glPushMatrix();{
		glColor3f(1,1,1);
		glTranslatef(7,0.1,10.5);
		glScalef(16,0.2,4.1);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);
		 

	}glPopMatrix();

	//middle upper 1 black glass1
	glPushMatrix();{
		glColor3f(0,0,0);
		glTranslatef(7,0,10);
		glScalef(16,0.2,0.7);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);
		 

	}glPopMatrix();

	//middle upper 1 black glass2
	glPushMatrix();{
		glColor3f(0,0,0);
		glTranslatef(7,0,14);
		glScalef(16,0.2,0.7);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);
		 

	}glPopMatrix();


		//middle upper 1 door border
	glPushMatrix();{
		glColor3f(0,0,0);
		glTranslatef(13,-0.3,0);
		glRotatef(45,0,1,0);
		glScalef(0.4,1,14);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);
		 
	}glPopMatrix();

		//middle upper 2 door border 2
	glPushMatrix();{
		glColor3f(0,0,0);
		glTranslatef(7,-0.3,0);
		glScalef(0.2,1,10);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);
		 
	}glPopMatrix();


		//middle upper 1 handle
	glPushMatrix();{
		glColor3f(1,1,1);
		glTranslatef(16,-0.2,7.5);
		glScalef(3,1,0.8);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);
		 
	}glPopMatrix();

		//middle upper 2
	glPushMatrix();{
		glColor3f(1,1,1);
		glTranslatef(7,19.7,10.5);
		glScalef(16,0.2,4.1);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);
		 
	}glPopMatrix();


		//middle upper 2 black glass1
	glPushMatrix();{
		glColor3f(0,0,0);
		glTranslatef(7,19.8,10);
		glScalef(16,0.2,0.7);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);
		 
	}glPopMatrix();


		//middle upper 2 black glass2
	glPushMatrix();{
		glColor3f(0,0,0);
		glTranslatef(7,19.8,14);
		glScalef(16,0.2,0.7);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);
		 
	}glPopMatrix();


		//middle upper 2 door border 1
	glPushMatrix();{
		glColor3f(0,0,0);
		glTranslatef(13,19.3,0);
		glRotatef(45,0,1,0);
		glScalef(0.4,1,14);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);
		 
	}glPopMatrix();

		//middle upper 2 door border 2
	glPushMatrix();{
		glColor3f(0,0,0);
		glTranslatef(7,19.3,0);
		glScalef(0.2,1,10);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);
		 
	}glPopMatrix();


		//middle upper 2 handle
	glPushMatrix();{
		glColor3f(1,1,1);
		glTranslatef(16,19.4,7.5);
		 glScalef(3,1,0.8);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);
		 
	}glPopMatrix();


	//glass front
	glPushMatrix();{

		glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(1,1,1,0.3);
	 
		glTranslatef(0,0.2,10);
		glRotatef(53,0,1,0);
		glScalef(.5,19.6,8.5);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);
	 glDisable(GL_BLEND);
	}glPopMatrix();

 

	//glass back
	glPushMatrix();{
		glColor3f(1,1,1);
		glTranslatef(30,0.2,10);
		glRotatef(-57,0,1,0);
		glScalef(.5,19.6,8.5);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);
	 
	}glPopMatrix();


		//glass back small
	glPushMatrix();{
		glColor3f(0,0,0);
		glTranslatef(30.5,0.2,10.1);
		glRotatef(-57,0,1,0);
		glScalef(.5,19.6,1);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);
	 
	}glPopMatrix();

	//upper triangle1
	glPushMatrix();{
		glColor3f(0,0,0);
		glTranslatef(1.5,0.3,8);
		glScalef(5,0,4.5);
		glTranslatef(1,1,0.5);
		glutSolidCone(1,1,20,20);
		 
	}glPopMatrix();

	//upper triangle2
	glPushMatrix();{
		glColor3f(0,0,0);
		glTranslatef(1.5,19.7,8);
		glScalef(5,0,4.5);
		glTranslatef(1,1,0.5);
		glutSolidCone(1,1,20,20);

	}glPopMatrix();

	//upper triangle3
	glPushMatrix();{
		glColor3f(0,0,0);
		glTranslatef(18.4,0.3,8);
		glScalef(5.25,0,4.5);
		glTranslatef(1,1,0.5);
		glutSolidCone(1,1,20,20);

	}glPopMatrix();

		//upper triangle4
	glPushMatrix();{
		glColor3f(0,0,0);
		glTranslatef(18.4,19.7,8);
		glScalef(5.25,0,4.5);
		glTranslatef(1,1,0.5);
		glutSolidCone(1,1,20,20);

	}glPopMatrix();

	//under back
	glPushMatrix();{
		glColor3f(.3,.3,.3);
		glTranslatef(30,0,0);
		glScalef(4,20,9.7);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);
	 

	}glPopMatrix();


	//under front
	glPushMatrix();{
		glColor3f(.3,.3,.3);
		//glTranslatef(30,0,0);
		glScalef(8,20,6.7);
		glTranslatef(-0.5,0.5,0.5);
		glutSolidCube(1);

	}glPopMatrix();

		//front rotate
	glPushMatrix();{
		 glColor3f(.3,.3,.3);
		glTranslatef(-8,0,6.7);
		glRotatef(67,0,1,0);
		glScalef(1,20,8.7);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);

	}glPopMatrix();

	//wheels1
	glPushMatrix();{
	wheel(0,0,0,0);
	}glPopMatrix();
	////wheels2
	glPushMatrix();{
	wheel(25,0.2,0,0);
	}glPopMatrix();
 
	////wheels3
	glPushMatrix();{
	 
	wheel(25,20,0,180);
	}glPopMatrix();
	 

	////wheels4
	glPushMatrix();{
	wheel(1,20,0,180);
	}glPopMatrix();
 
	//back light 1
	glPushMatrix();{
		light(0);
	}glPopMatrix();

	//back light 2
	glPushMatrix();{
		light(13);
	}glPopMatrix();


	//cylender 1
	glPushMatrix();{
		glColor3f(.75,.75,.75);
		glTranslatef(26,14,-1);
		glScalef(10,3,1.5);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);
			glLineWidth(2);
		glColor3f(0,0,0);
		glutWireCube(1);
	}glPopMatrix();

	
	//cylender 2
	glPushMatrix();{
		glColor3f(.75,.75,.75);
		glTranslatef(26,3,-1);
		glScalef(10,3,1.5);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);
			glLineWidth(2);
		glColor3f(0,0,0);
		glutWireCube(1);
	}glPopMatrix();

	//front light 1

	glPushMatrix();{

		glColor3f(1,1,.6);
		glTranslatef(-7.8,3,4.5);
		glutSolidSphere(1.5,20,20);

	}glPopMatrix();

	//front light 2

	glPushMatrix();{

		glColor3f(1,1,.6);
		glTranslatef(-7.8,17,4.5);
		glutSolidSphere(1.5,20,20);

	}glPopMatrix();

	//looking glass 1
	glPushMatrix();
	lookingglass();
	glPopMatrix();

	//looking glass 2
	glPushMatrix();
	lookingglass2();
	glPopMatrix();

	//logo

	glPushMatrix();{
	glTranslatef(34.2,7,1.3);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,bmw);


	glColor3f(1,1,1);
	glBegin(GL_QUADS);
	glTexCoord2f(0,1);
	glVertex3f(0,0,0);
	glTexCoord2f(0,0);
	glVertex3f(0,6,0);
		glTexCoord2f(1,0);
	glVertex3f(0,6,3);
		glTexCoord2f(1,1);
	glVertex3f(0,0,3);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	}glPopMatrix();


	//back upper loght

	glPushMatrix();{
		glColor3f(0.1,0.1,0.1);
		glTranslatef(33.3,0.3,8.8);
		glScalef(1,19.5,0.8);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);
	}glPopMatrix();



}


/////////////////////////////////////////////////////////////////////////////////

void gridAndAxis(){
	// draw the three major AXES
	if(canDrawAxis==1){
		glBegin(GL_LINES);
			//X axis
			glColor3f(0, 1, 0);	//100% Green
			glVertex3f(-150, 0, 0);
			glVertex3f( 150, 0, 0);
		
			//Y axis
			glColor3f(0, 0, 1);	//100% Blue
			glVertex3f(0, -150, 0);	// intentionally extended to -150 to 150, no big deal
			glVertex3f(0,  150, 0);
		
			//Z axis
			glColor3f(1, 1, 1);	//100% White
			glVertex3f( 0, 0, -150);
			glVertex3f(0, 0, 150);
		glEnd();
	}

	if(canDrawGrid == 1){
	//some gridlines along the field
		int i;

		glColor3f(0.5, 0.5, 0.5);	//grey
		glBegin(GL_LINES);
			for(i=-10;i<=10;i++){

				if(i==0)
					continue;	//SKIP the MAIN axes

				//lines parallel to Y-axis
				glVertex3f(i*10, -100, 0);
				glVertex3f(i*10,  100, 0);

				//lines parallel to X-axis
				glVertex3f(-100, i*10, 0);
				glVertex3f( 100, i*10, 0);
			}
		glEnd();

	}
}


void road(int x){
	glTranslatef(-300,-100,-0.2);
	glScalef(1,x,1);
glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, grassimg);

		glColor3f(1,1,1);
		glBegin(GL_QUADS);
			glTexCoord2f(0,0);
			glVertex3f(0,0,0);
			glTexCoord2f(1,0);
			glVertex3f(600,0,0);
			glTexCoord2f(1,1);
			glVertex3f(600,6000,0);
			glTexCoord2f(0,1);
			glVertex3f(0,6000,0);
		glEnd();

	glDisable(GL_TEXTURE_2D);

}

void roadblack(int x){

		glTranslatef(-200,-100,0.3);
	glScalef(1,x,1);
glPushMatrix();{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,roadmain);
	glColor3f(1,1,1);
	glBegin(GL_QUADS);
	glTexCoord2f(1,0);
	glVertex3f(0,0,0);
	glTexCoord2f(0,0);
	glVertex3f(400,0,0);
	glTexCoord2f(0,1);
	glVertex3f(400,6000,0);
	glTexCoord2f(1,1);
    glVertex3f(0,6000,0);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	}glPopMatrix();

}

void roadviewleft(int x){
	glTranslatef(-695,x,1);
	glPushMatrix();{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,sceenarioleft);

	glColor3f(1,1,1);
	glBegin(GL_QUADS);
	glTexCoord2f(0,0);
	glVertex3f(0,0,150);
	glTexCoord2f(1,0);
	glVertex3f(400,0,0);
	glTexCoord2f(0,1);
	glVertex3f(400,1000,0);
	glTexCoord2f(1,1);
    glVertex3f(0,1000,150);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	}
	glPopMatrix();

}


void roadviewright(int x){

	glTranslatef(295,x,1);
	glPushMatrix();{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,sceenarioright);
	glColor3f(1,1,1);
	glBegin(GL_QUADS);
	glTexCoord2f(0,0);
	glVertex3f(0,0,0);
	glTexCoord2f(1,0);
	glVertex3f(300,0,100);
	glTexCoord2f(0,1);
	glVertex3f(300,1000,100);
	glTexCoord2f(1,1);
    glVertex3f(0,1000,0);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	}glPopMatrix();

}

void biondaryleft(int x,int y){
	glColor3f(1,.6,.2);
	glTranslatef(y,0,0);
	glPushMatrix();{
		glScalef(4,x,6);
		glTranslatef(-0.5,0.5,0.5);
		glutSolidCube(1);
		glColor3f(0.5,0,0);
		glutWireCube(1);
	}glPopMatrix();


}


void biondarymiddle(int x,int y){
	glColor3f(1,.6,.2);
	glTranslatef(y,0,0.9);
	glPushMatrix();{
		glScalef(5,x,1);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);
		 
	}glPopMatrix();


}




void home(double s,double r,double ty,double tx,double cx,double cy,double cz){
	glTranslatef(tx,ty,0);
	glScalef(1,1,s);
	glRotatef(r,0,0,1);

		//window 1
	 glColor3f(.7	,.7	,.7);
	glBegin(GL_QUADS);{
	
		glVertex3f(5,-.5,10);
		glVertex3f(15,-.5,10);
		glVertex3f(15,-.5,25);
		glVertex3f(5,-.5,25);
	
	}glEnd();
	//111
	 glColor3f(.2	,.2	,.2);
	glBegin(GL_QUADS);{
	
		glVertex3f(6,-.9,12);
		glVertex3f(9,-.9,12);
		glVertex3f(9,-.9,16);
		glVertex3f(6,-.9,16);
	
	}glEnd();
	//222
	 glColor3f(.2	,.2	,.2);
	glBegin(GL_QUADS);{
	
		glVertex3f(11,-.9,12);
		glVertex3f(14,-.9,12);
		glVertex3f(14,-.9,16);
		glVertex3f(11,-.9,16);
	
	}glEnd();

	//111
	 glColor3f(.2	,.2	,.2);
	glBegin(GL_QUADS);{
	
		glVertex3f(6,-.9,18);
		glVertex3f(9,-.9,18);
		glVertex3f(9,-.9,23);
		glVertex3f(6,-.9,23);
	
	}glEnd();
	//222
	 glColor3f(.2	,.2	,.2);
	glBegin(GL_QUADS);{
	
		glVertex3f(11,-.9,18);
		glVertex3f(14,-.9,18);
		glVertex3f(14,-.9,23);
		glVertex3f(11,-.9,23);
	
	}glEnd();

	//window 2
  glColor3f(.7	,.7	,.7);
	glBegin(GL_QUADS);{
		 
		glVertex3f(25,-.5,10);
		glVertex3f(35,-.5,10);
		glVertex3f(35,-.5,25);
		glVertex3f(25,-.5,25);
	
	}glEnd();

	//111
	 glColor3f(.2	,.2	,.2);
	glBegin(GL_QUADS);{
	
		glVertex3f(26,-.9,11);
		glVertex3f(29,-.9,11);
		glVertex3f(29,-.9,16);
		glVertex3f(26,-.9,16);
	
	}glEnd();
	//222
	 glColor3f(.2	,.2	,.2);
	glBegin(GL_QUADS);{
	
		glVertex3f(31,-.9,11);
		glVertex3f(34,-.9,11);
		glVertex3f(34,-.9,16);
		glVertex3f(31,-.9,16);
	
	}glEnd();
	//111
	 glColor3f(.2	,.2	,.2);
	glBegin(GL_QUADS);{
	
		glVertex3f(26,-.9,18);
		glVertex3f(29,-.9,18);
		glVertex3f(29,-.9,23);
		glVertex3f(26,-.9,23);
	
	}glEnd();
	//222
	 glColor3f(.2	,.2	,.2);
	glBegin(GL_QUADS);{
	
		glVertex3f(31,-.9,18);
		glVertex3f(34,-.9,18);
		glVertex3f(34,-.9,23);
		glVertex3f(31,-.9,23);
	
	}glEnd();
	

	//door
	     glPushMatrix();{
	glColor3f(.7	,.7	,.7);
	 
	glTranslatef(40,10,12);
	glScalef(1,6,18);
	
	glutSolidCube(1);
	glColor3f(.2	,.2	,.2);
	glutWireCube(1);
	 

	}glPopMatrix();

		 //door window 1
	     glPushMatrix();{
	glColor3f(.2	,.2	,.2);
	 
	glTranslatef(40,3,16);
	glScalef(1,3,10);
	
	glutSolidCube(1);
	 

	}glPopMatrix();

		 //door window 2
	     glPushMatrix();{
	glColor3f(.2	,.2	,.2);
	 
	glTranslatef(40,17,16);
	glScalef(1,3,10);
	
	glutSolidCube(1);
	 

	}glPopMatrix();

		 	//pilar 1
	     glPushMatrix();{
	glColor3f(.7	,.7	,.7);
	 
	glTranslatef(48,1,20);
	glScalef(2,1,28);
	
	glutSolidCube(1);
	 
	 

	}glPopMatrix();

		  	//pilar 2
	     glPushMatrix();{
	glColor3f(.7	,.7	,.7);
	 
	glTranslatef(48,7,20);
	glScalef(2,1,28);
	
	glutSolidCube(1);
	 
	 

	}glPopMatrix();

		  	//pilar 3
	     glPushMatrix();{
	glColor3f(.7	,.7	,.7);
	 
	glTranslatef(48,15,20);
	glScalef(2,1,28);
	
	glutSolidCube(1);
	 
	 

	}glPopMatrix();

		  	//pilar 4
	     glPushMatrix();{
	glColor3f(.7	,.7	,.7);
	 
	glTranslatef(48,19,20);
	glScalef(2,1,28);
	
	glutSolidCube(1);
	 
	 

	}glPopMatrix();

		 	  	//pilar middle
	     glPushMatrix();{
	glColor3f(.7	,.7	,.7);
	 
	glTranslatef(48,10,34);
	glScalef(2,20,2);
	
	glutSolidCube(1);
	 
	 

	}glPopMatrix();

 

		 //roof wall 1
      glPushMatrix();{
	 glColor3f(.7	,.7	,.7);
	glRotatef(45,1,0,0);
	glTranslatef(25,40,16);
	glScalef(50,2,18);
	
	glutSolidCube(1);
	/*glColor3f(0,1,0);
	glutWireCube(1);*/

	}glPopMatrix();


	  	 //roof wall 2
      glPushMatrix();{
	 glColor3f(.7	,.7	,.7);
	glRotatef(-45,1,0,0);
	glTranslatef(25,-26,32);
	glScalef(50,2,18);
	
	glutSolidCube(1);
	/*glColor3f(0,1,0);
	glutWireCube(1);*/

	}glPopMatrix();

	//uper wall
      glPushMatrix();{
	glColor3f(cx	,cy	,cz);
	//glRotatef(-40,0,0,1);
	glTranslatef(20,10,21);
	glScalef(40,20,30);
	
	glutSolidCube(1);
	/*glColor3f(0,1,0);
	glutWireCube(1);*/

	}glPopMatrix();
	  
	  
	  //under wall

	    glPushMatrix();{
	glColor3f(.7	,.7	,.7);
	//glRotatef(-40,0,0,1);
	glTranslatef(25,10,3);
	glScalef(50,20,6);
	
	glutSolidCube(1);
	/*glColor3f(1,0,1);
	glutWireCube(1);*/

	}glPopMatrix();


		//roof side 1

		glPushMatrix();{
			glTranslatef(48,10,10);
		 glRotatef(180,0,0,1);
	double equ[4];
	equ[0]=1;
	equ[1]=0;
	equ[2]=0;
	equ[3]=0;

	glClipPlane(GL_CLIP_PLANE0,equ);

	glEnable(GL_CLIP_PLANE0);{
	glTranslatef(-10,0,24);
	 glColor3f(1	,.31	,.31);
	glutSolidCone(13.5,45,20,20);
 
	}glDisable(GL_CLIP_PLANE0);

	}glPopMatrix();


		//roof side 2
glPushMatrix();{
		double equ[4];
	equ[0]=1;
	equ[1]=0;
	equ[2]=0;
	equ[3]=0;

	glClipPlane(GL_CLIP_PLANE0,equ);

	glEnable(GL_CLIP_PLANE0);{
	glTranslatef(-10,10,35);
	  glColor3f(1	,.31	,.31);
	glutSolidCone(15,30,20,20);
 
	}glDisable(GL_CLIP_PLANE0);

	}glPopMatrix();


         //roof side window
 
	     glPushMatrix();{
	glColor3f(.2	,.2	,.2);
	 
	glTranslatef(47,10,39);
	glScalef(1,3,5);
	
	glutSolidCube(1);
	 

	}glPopMatrix();


		 //sheree wall left
		 glPushMatrix();{
		 	GLUquadricObj *obj=gluNewQuadric();
	 glTranslatef(48,6,0);
	for (int i = 10; i > 0; i--)
	{
		 
		if(i%2==0){
			  glColor3f(1	,.31	,.31);
		}else
		{
			 glColor3f(.7	,.7	,.7);
		}
		
		glTranslatef(1,0,0);
		gluCylinder(obj,.5,.5,5+i,20,20);
 
	}
	}glPopMatrix();

		  //sheree wall right
		 	 glPushMatrix();{
		 	GLUquadricObj *obj=gluNewQuadric();
	 glTranslatef(48,15,0);
	for (int i = 10; i > 0; i--)
	{
		 
		if(i%2==0){
		 glColor3f(1	,.31	,.31);
		}else
		{
		 glColor3f(.7	,.7	,.7);
		}
		
		glTranslatef(1,0,0);
		gluCylinder(obj,.5,.5,5+i,20,20);
 
	}
	}glPopMatrix();


			   //sheree 1
		 	 glPushMatrix();{
		 glColor3f(1	,.31	,.31);
		  glTranslatef(51,10.5,3);
		glScalef(2,9,6);
		glutSolidCube(1);
	}glPopMatrix();

			    //sheree 2
		 	 glPushMatrix();{
		 glColor3f(.7	,.7	,.7);
		  glTranslatef(53,10.5,2);
		glScalef(2,9,4);
		glutSolidCube(1);
	}glPopMatrix();

			     //sheree 3
		 	 glPushMatrix();{
		 glColor3f(1	,.31	,.31);
		  glTranslatef(55,10.5,1);
		glScalef(2,9,2);
		glutSolidCube(1);
	}glPopMatrix();

			 	    //sheree 4
		 	 glPushMatrix();{
		 glColor3f(.7	,.7	,.7);
		  glTranslatef(57,10.5,0.5);
		glScalef(2,9,1);
		glutSolidCube(1);
	}glPopMatrix();
 


}



void lampost(int tx,int ty,int r){

	glTranslatef(tx,ty,0);
	glScalef(1.2,1.2,2);
		glRotatef(r,0,0,1);
		glPushMatrix();
		glTranslatef(0,0,0);
	
glEnable(GL_TEXTURE_2D);
glBindTexture(GL_TEXTURE_2D,cylender);
	glColor3f(1,1,1);
	GLUquadricObj *sphere=gluNewQuadric();
	gluQuadricTexture(sphere,GL_TRUE);
	 
	gluCylinder(sphere,1,1,60,20,20);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();


	glPushMatrix();{
		glColor3f(0.1,0.1,0.1);
		glTranslatef(-1,0,58);
		
		glRotatef(-15,0,1,0);
		glScalef(30,2,2);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);
	}glPopMatrix();

	glPushMatrix();{
		glColor3f(1,1,0.6);
		glTranslatef(17,0,62.2);
	 
		glRotatef(-15,0,1,0);
		glScalef(10,1,1);
		glTranslatef(0.5,0.5,0.5);
		glutSolidCube(1);
	}glPopMatrix();


}

  //ball

void ball(double tx,int ty,double c1,double c2,double c3){
	glPushMatrix();{
		glColor3f(c1,c2,c3);
		glTranslatef(tx,ty,1+ballty);
	glScalef(4,2,5);
	glTranslatef(1,1,1);
	glutSolidSphere(1,20,20);
	}glPopMatrix();
}

/********************************************************************************/

void display(){
	//codes for Models, Camera
	
	//clear the display
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0,0,0,0);	//color black
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		//clear buffers to preset values

	/***************************
	/ set-up camera (view) here
	****************************/ 
	//load the correct matrix -- MODEL-VIEW matrix
	glMatrixMode(GL_MODELVIEW);		//specify which matrix is the current matrix

	//initialize the matrix
	glLoadIdentity();				//replace the current matrix with the identity matrix [Diagonals have 1, others have 0]

	//now give three info
	//1. where is the camera (viewer)?
	//2. where is the camera looking?
	//3. Which direction is the camera's UP direction?

	//gluLookAt(0,-150,20,	0,0,0,	0,0,1);
	//gluLookAt(cameraRadius*sin(cameraAngle), -cameraRadius*cos(cameraAngle), cameraHeight,		0,0,0,		0,0,1);

	V to=loc+dir;
	gluLookAt(loc.x, loc.y, loc.z,		to.x,to.y,to.z,		perp.x,perp.y,perp.z);
	
	//again select MODEL-VIEW
	glMatrixMode(GL_MODELVIEW);


	/**************************************************
	/ Grid and axes Lines(You can remove them if u want)
	***************************************************/
	
	//gridAndAxis();
	
	
	/****************************
	/ Add your objects from here
	****************************/
	//car
	glPushMatrix();{
	car(transletx,translety,transletz,carrotation);
	}glPopMatrix();


	//road
	glPushMatrix();{
	road(1);
	}glPopMatrix();
	
	//road main
	glPushMatrix();{
		roadblack(1);
	}glPopMatrix();

	//road view left

	glPushMatrix();
	roadviewleft(0);
	glPopMatrix();

	glPushMatrix();
	roadviewleft(1000);
	glPopMatrix();

	glPushMatrix();
	roadviewleft(2000);
	glPopMatrix();

		glPushMatrix();
	roadviewleft(3000);
	glPopMatrix();

	glPushMatrix();
	roadviewleft(4000);
	glPopMatrix();
	glPushMatrix();
	roadviewleft(5000);
	glPopMatrix();
	 
	//road view right

	glPushMatrix();
	roadviewright(0);
	glPopMatrix();

	glPushMatrix();
	roadviewright(1000);
	glPopMatrix();

	glPushMatrix();
	roadviewright(2000);
	glPopMatrix();
	glPushMatrix();
	roadviewright(3000);
	glPopMatrix();

	glPushMatrix();
	roadviewright(4000);
	glPopMatrix();
	glPushMatrix();
	roadviewright(5000);
	glPopMatrix();

	//road left bioundary
	glPushMatrix();
	biondaryleft(6000,-200);
	glPopMatrix();

		//road right bioundary
	glPushMatrix();
	biondaryleft(6000,201);
	glPopMatrix();

	//home left
	glPushMatrix();

	home(1,40,400,-270,0.3,0.2,1);
	glPopMatrix();

	glPushMatrix();

	home(1.1,40,2400,-270,0.6,0.8,.1);
	glPopMatrix();

	glPushMatrix();

	home(1.2,40,5000,-270,0.3,0.7,.3);
	glPopMatrix();


	//home right
		glPushMatrix();

	home(1,-80,1500,230,0.9,0.2,0.5);
	glPopMatrix();

		glPushMatrix();

	home(1.2,-80,4000,230,0.1,0.6,1);
	glPopMatrix();

		glPushMatrix();

	home(1.2,-80,5700,230,0.73,0.928,0.236);
	glPopMatrix();

	// lampost

	for(int i=0; i<6000; i=i+200)
	{
	glPushMatrix();
	lampost(-210,i,0);
	glPopMatrix();

	glPushMatrix();
	lampost(210,i,180);
	glPopMatrix();

	}

	//middle line

	glPushMatrix();{
		biondarymiddle(6000,10);
	}glPopMatrix();

	//point show
	if(point<=-600){

			loc=_L,dir=_D,perp=_P;
			speed=6;
			loc=loc+4*speed*perp;
	loc=loc+speed*((dir*perp).unit());
			translety=-1;

		glPushMatrix();{
	std::string text1;
    std::string text;
	text1 ="YOU LOST PRESS R To START AGAIN";
 
	text = text1;

   glColor3f(1,.31,.31);
    
   drawText(text.data(), text.size(), 400, 530);
	}glPopMatrix();

	}else{

	glPushMatrix();{
	std::string text1;
    std::string text;
	text1 ="POINTS :: ";
 
	text = text1 + std::to_string((int)point);

   glColor3f(1,1,0);
    
   drawText(text.data(), text.size(), 550, 530);
	}glPopMatrix();

	

		glPushMatrix();{
	std::string text1;
    std::string text;
	text1 ="DISTANCE : ";
 
	text = text1 + std::to_string((int)translety);

   glColor3f(1,1,0);
    
   drawText(text.data(), text.size(), 550, 500);
	}glPopMatrix();
	}

	// ball
	srand(time(NULL));
	for(int i=500;i<=6000; i=i+500){
	
	int r = rand() % 10;
	int r2=rand()%10;
	int r3=rand()%10;
 
	glPushMatrix();{
		
		ball(9.2,i,r*0.1,r2*0.1,r3*0.1);
	}glPopMatrix();
	}


	
	// ball left
	 
	for(int i=700;i<=6000; i=i+700){
	
	int r = rand() % 10;
	int r2=rand()%10;
	int r3=rand()%10;
 
	glPushMatrix();{
		
		ball(9.2-80,i,r*0.1,r2*0.1,r3*0.1);
	}glPopMatrix();
	}



	
	// ball right
	for(int i=800;i<=6000; i=i+800){
		 
	int r = rand() % 10;
	int r2=rand()%10;
	int r3=rand()%10;
 
	glPushMatrix();{
		
		ball(9.2+80,i,r*0.1,r2*0.1,r3*0.1);
	}glPopMatrix();
	}

	//ADD this line in the end --- if you use double buffer (i.e. GL_DOUBLE)
	glFlush();
	glutSwapBuffers();
}

void animate(){
	//codes for any changes in Models, Camera

 
	if(jump==1){

		carspeed=1.2;
		speed=1.2;

		if(transletz>15){
			jumpi=1;
		}
		if (jumpi==1)
		{
			transletz-=0.7;
			if (transletz<=0)
			{
				jumpi=0;
				jump=0;
			}
		}

		transletz+=0.4;
		 

		///condition right up
		
		if(translety>translateyalarm && translety<6000){
			if(carrotation==0){
				loc=loc+speed*dir;
				translety+=carspeed;
				point+=carspeed;
			}else if(carrotation<0 && carrotation>=-70){
				 
				loc=loc+speed*dir;
				transletx+=carrotation*(-0.01*carspeed);
				translety+=carspeed;
				point+=carspeed;
			}else if (carrotation<-70 && carrotation>=-85)
			{
				speed=0.4;
			 
				 loc=loc+speed*((dir*perp).unit());
				transletx+=carrotation*(-0.01*carspeed);
				translety+=0.1*carspeed;
				point+=0.1*carspeed;
			}
			else if (carrotation<-85 && carrotation>=-90)
			{
				speed=0.5;
				  loc=loc+speed*((dir*perp).unit());
				transletx+=carrotation*(-0.01*carspeed);
				translety+=0;
				point+=0;
			}


			//condition left up

			 
			if(carrotation>0 && carrotation<=70){
			 
				loc=loc+speed*dir;
				transletx+=carrotation*(-0.01*carspeed);
				translety+=carspeed;
				point+=carspeed;
			}else if (carrotation>70 && carrotation<=85)
			{
				 speed=0.4;
			 
				loc=loc+speed*((perp*dir).unit());
				transletx+=carrotation*(-0.01*carspeed);
				translety+=0.1*carspeed;
				point+=0.1*carspeed;
			}
			else if (carrotation>85 && carrotation<=90)
			{
				speed=0.5;
			 loc=loc+speed*((perp*dir).unit());
				transletx+=carrotation*(-0.01*carspeed);
				translety+=0;
				point+=0;
			}

		}

	}
	

	if(ballty>8){
		balli=1;
	}
	if (balli==1)
	{
		ballty-=0.4;
		if(ballty<0){
          balli=0;
		}
	}
	ballty+=0.2;
	 


	
	if(transletx<=-185){
		transletx=-185;
		translateyalarm=translety;
	}else if(transletx>=170){
		transletx=170;
		translateyalarm=translety;
	}else{
		translateyalarm=-1;
	}

	

	if(translety>translateyalarm && translety<6000){
		speed=0.5;
		carspeed=0.5;
		


		if(carrotation>=90){
				carrotation=0;
				translety+=9;
				point+=9;
			}

			if(carrotation<=-90){
				carrotation=0; //22.5*0.4
				translety-=9;
				point-=9;
			}




		if(carrotation<=0){
		wheelrotate+=2;
		}else{
			wheelrotate-=2;
		}
			///condition right up

			if(carrotation==0){
				loc=loc+speed*dir;
				translety+=carspeed;
				point+=carspeed;
			}else if(carrotation<0 && carrotation>=-70){
				 
				loc=loc+speed*dir;
				transletx+=carrotation*(-0.01*carspeed);
				translety+=carspeed;
				point+=carspeed;
			}else if (carrotation<-70 && carrotation>=-85)
			{
				speed=0.4;
			 
				 loc=loc+speed*((dir*perp).unit());
				transletx+=carrotation*(-0.01*carspeed);
				translety+=0.1*carspeed;
				point+=0.1*carspeed;
			}
			else if (carrotation<-85 && carrotation>=-90)
			{
				speed=0.5;
				  loc=loc+speed*((dir*perp).unit());
				transletx+=carrotation*(-0.01*carspeed);
				translety+=0;
				point+=0;
			}


			//condition left up

			 
			if(carrotation>0 && carrotation<=70){
			 
				loc=loc+speed*dir;
				transletx+=carrotation*(-0.01*carspeed);
				translety+=carspeed;
				point+=carspeed;
			}else if (carrotation>70 && carrotation<=85)
			{
				 speed=0.4;
			 
				loc=loc+speed*((perp*dir).unit());
				transletx+=carrotation*(-0.01*carspeed);
				translety+=0.1*carspeed;
				point+=0.1*carspeed;
			}
			else if (carrotation>85 && carrotation<=90)
			{
				speed=0.5;
			 loc=loc+speed*((perp*dir).unit());
				transletx+=carrotation*(-0.01*carspeed);
				translety+=0;
				point+=0;
			}



	}

	

	//cameraAngle += cameraAngleDelta;	// camera will rotate at 0.002 radians per frame.
	
	//codes for any changes in Models

	//MISSING SOMETHING? -- YES: add the following
	glutPostRedisplay();	//this will call the display AGAIN

}

void init(){
	//codes for initialization
	////////////////////////////////
	jump=0;
	point=0;
	bonus=0;
	translateyalarm=-1;
	carspeed=6;
	wheelrotate=0;
	transletx=0;
	translety=-1;
	transletz=0;
	carrotation=0;
	ballty=0;
	///////////////////////////////////////////
	loadImage();
	
	cameraAngle = 0;	//angle in radian
	move_X = 0;
	move_Y = 0;
	move_Z = 0;
	canDrawGrid = 1;
	canDrawAxis = 1;

	cameraAngleDelta = .001;

	cameraRadius = 150;
	cameraHeight = 50;

	
	loc=_L,dir=_D,perp=_P;
	loc=loc+4*speed*perp;
	loc=loc+speed*((dir*perp).unit());
 
	//clear the screen
	glClearColor(0,0,0, 0);

	/************************
	/ set-up projection here
	************************/
	//load the PROJECTION matrix
	glMatrixMode(GL_PROJECTION);
	
	//initialize the matrix
	glLoadIdentity();

	/*
		gluPerspective() — set up a perspective projection matrix

		fovy -         Specifies the field of view angle, in degrees, in the y direction.
        aspect ratio - Specifies the aspect ratio that determines the field of view in the x direction. The aspect ratio is the ratio of x (width) to y (height).
        zNear -        Specifies the distance from the viewer to the near clipping plane (always positive).
        zFar  -        Specifies the distance from the viewer to the far clipping plane (always positive).
    */
	
	gluPerspective(70,	1,	0.1,	10000.0);
	
}

void keyboardListener(unsigned char key, int x,int y){
	switch(key){
	
	case 'w':case 'W':
		speed=6;
		loc=loc+speed*dir;
		break;

	case 'a':case 'A':
		speed=6;
		loc=loc+speed*((perp*dir).unit());
		break;

	case 's':case 'S':
		speed=6;
		loc=loc-speed*dir;
		break;

	case 'd':case 'D':
		speed=6;
		loc=loc+speed*((dir*perp).unit());
		break;

	case 'q':case 'Q':
		perp=perp.rot(dir,ang_speed);
		break;

	case 'e':case 'E':
		perp=perp.rot(-dir,ang_speed);
		break;

		case 'r':case 'R':
	  loc=_L,dir=_D,perp=_P;
	  speed=6;
	  loc=loc+4*speed*perp;
	  loc=loc+speed*((dir*perp).unit());
	  wheelrotate=0;
	transletx=0;
	translety=0;
	transletz=0;
	carrotation=0;
	point=0;
		break;
	
		case 'f':case 'F':
			loc=_L,dir=_D,perp=_P;
			speed=6;
			loc=loc+4*speed*perp;
	loc=loc+speed*((dir*perp).unit());
			translety=-1;
			point=0;
			carrotation=180;
			break;

		case 'j':case 'J':
			if (translety>=0)
			{
			jump=1;
			}
			break;
 
	default:
		break;
	}
}

void specialKeyListener(int key, int x,int y){
	V p;

	switch(key){
	case GLUT_KEY_UP:
		
		if(translety<=translateyalarm){
		}else
		{

		


		if(carrotation>=90){
				carrotation=0;
				translety+=9;
				point+=9;
			}

			if(carrotation<=-90){
				carrotation=0; //22.5*0.4
				translety-=9;
				point-=9;
			}




		if(carrotation<=0){
		wheelrotate+=10;
		}else{
			wheelrotate-=10;
		}
			///condition right up

		if((transletx>=-90 && transletx<=-65 && ((int)translety%700<=7) && transletz<=10) || (transletx>=-18 && transletx<=18 && ((int)translety%500<=6) && transletz<=10) || (transletx>=67 && transletx<=95 && ((int)translety%800<=10) && transletz<=10) ){
		
			point-=200;
						speed=0;
		carspeed=0;
		loc=loc+speed*dir;
				translety+=carspeed;
		}else
		{
			bonus=0;


			if(carrotation==0){
					speed=6;
		carspeed=6;
		loc=loc+speed*dir;
				translety+=carspeed;
				point+=carspeed;
			}else if(carrotation<0 && carrotation>=-70){
			
		 	speed=6;
		carspeed=6;
		loc=loc+speed*dir;

				transletx+=carrotation*(-0.01*carspeed);
				translety+=carspeed;
				point+=carspeed;
			}else if (carrotation<-70 && carrotation>=-85)
			{
		   speed=5;
		   carspeed=5;
		     loc=loc+speed*((dir*perp).unit());
				transletx+=carrotation*(-0.01*carspeed);
				translety+=0.1*carspeed;
				point+=0.1*carspeed;
			}
			else if (carrotation<-85 && carrotation>=-90)
			{
				speed=6;
				carspeed=6;
		     loc=loc+speed*((dir*perp).unit());
				transletx+=carrotation*(-0.01*carspeed);
				translety+=0;
				point+=0;
			}


			//condition left up

			 
			if(carrotation>0 && carrotation<=70){
			
		 	speed=6;
		carspeed=6;
		loc=loc+speed*dir;

				transletx+=carrotation*(-0.01*carspeed);
				translety+=carspeed;
				point+=carspeed;
			}else if (carrotation>70 && carrotation<=85)
			{
					speed=5;
					carspeed=5;
		loc=loc+speed*((perp*dir).unit());
				transletx+=carrotation*(-0.01*carspeed);
				translety+=0.1*carspeed;
				point+=0.1*carspeed;
			}
			else if (carrotation>85 && carrotation<=90)
			{
					speed=6;
					carspeed=6;
		loc=loc+speed*((perp*dir).unit());
				transletx+=carrotation*(-0.01*carspeed);
				translety+=0;
				point+=0;
			}
		}

		}


	/*	p=(dir*perp).unit();
	 
		dir=dir.rot(p,ang_speed);
		perp=perp.rot(p,ang_speed);*/
		break;
	case GLUT_KEY_DOWN:
		speed=3;
		carspeed=3;
		loc=loc-speed*dir;





		if(carrotation>=90){
				carrotation=0;
				translety+=9;
				point+=9;
			}

			if(carrotation<=-90){
				carrotation=0; //22.5*0.4
				translety-=9;
				point-=9;
			}











		if(carrotation<=0){
			wheelrotate-=10;
			}else{
				wheelrotate+=10;
			}
			//condition left down
			if(carrotation==0){
				translety-=carspeed;
				point-=carspeed;
			}else if(carrotation<0 && carrotation>=-70){
			
				transletx+=carrotation*(0.01*carspeed);
				translety-=carspeed;
				point-=carspeed;
			}else if (carrotation<-70 && carrotation>=-85)
			{
				transletx+=carrotation*(0.01*carspeed);
				translety-=0.1*carspeed;
				point-=0.1*carspeed;
			}
			else if (carrotation<-85 && carrotation>=-90)
			{
				transletx+=carrotation*(0.01*carspeed);
				translety-=0;
				point-=0;
			}

			//condition right down

		 	if(carrotation>0 && carrotation<=70){
			
				transletx+=carrotation*(0.01*carspeed);
				translety-=carspeed;
				point-=carspeed;
			}else if (carrotation>70 && carrotation<=85)
			{
				transletx+=carrotation*(0.01*carspeed);
				translety-=0.1*carspeed;
				point-=0.1*carspeed;
			}
			else if (carrotation>85 && carrotation<=90)
			{
				transletx+=carrotation*(0.01*carspeed);
				translety-=0;
				point-=0;
			}





		/*p=(perp*dir).unit();
		dir=dir.rot(p,ang_speed);
		perp=perp.rot(p,ang_speed);*/
		break;

	case GLUT_KEY_LEFT:
	 
		speed=3;
		loc=loc+speed*((perp*dir).unit());
		dir=dir.rot(perp,ang_speed);


		wheelrotate-=5;
			if(carrotation>=90){
				carrotation=0;
				translety+=9;
				point+=9;
			}
			carrotation+=4;
			translety-=0.4;
			point-=0.4;



		break;
	case GLUT_KEY_RIGHT:
		 speed=3;
		loc=loc+speed*((dir*perp).unit());
		dir=dir.rot(-perp,ang_speed);


			wheelrotate+=5;
			if(carrotation<=-90){
				carrotation=0; //22.5*0.4
				translety-=9;
				point-=9;
			}
			carrotation-=4;
			translety+=0.4;
			point+=0.4;


		break;

	case GLUT_KEY_PAGE_UP:
		speed=6;
		loc=loc+speed*perp;
		break;
	case GLUT_KEY_PAGE_DOWN:
		speed=6;
		loc=loc-speed*perp;
		break;

	case GLUT_KEY_INSERT:
		break;

	case GLUT_KEY_HOME:
		loc=_L,dir=_D,perp=_P;
		break;
	case GLUT_KEY_END:
		break;

	default:
		break;
	}
}

void mouseListener(int button, int state, int x, int y){	//x, y is the x-y of the screen (2D)
	switch(button){
		case GLUT_LEFT_BUTTON:
			if(state == GLUT_DOWN){		// 2 times?? in ONE click? -- solution is checking DOWN or UP
				
			}
			break;

		case GLUT_RIGHT_BUTTON:
			//........
			break;

		case GLUT_MIDDLE_BUTTON:
			//........
			break;

		default:
			break;
	}
}

int main(int argc, char **argv){
	
	glutInit(&argc,argv);							//initialize the GLUT library
	
	glutInitWindowSize(600, 650);
	glutInitWindowPosition(100, 50);
	
	/*
		glutInitDisplayMode - inits display mode
		GLUT_DOUBLE - allows for display on the double buffer window
		GLUT_RGBA - shows color (Red, green, blue) and an alpha
		GLUT_DEPTH - allows for depth buffer
	*/
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);	

	glutCreateWindow("Some Title");

	printf("Camera Control\n");
	printf("_____________\n");
	printf("Roll: UP and DOWN arrow\n");
	printf("Pitch: Q and E\n");
	printf("Yaw: LEFT and RIGHT arrow\n");
	printf("Up-Down: PAGEUP and PAGEDOWN\n");
	printf("Left-Right: A and D\n");
	printf("Zoom in-out: W and S\n");
	printf("Front Look: F\n");
    printf("Jump Car: J\n");
	printf("Restart Game: R\n");
 

	init();						//codes for initialization

	glEnable(GL_DEPTH_TEST);	//enable Depth Testing

	glutDisplayFunc(display);	//display callback function
	glutIdleFunc(animate);		//what you want to do in the idle time (when no drawing is occuring)

	glutKeyboardFunc(keyboardListener);
	glutSpecialFunc(specialKeyListener);

	glutMouseFunc(mouseListener);

	glutMainLoop();		//The main loop of OpenGL

	return 0;
}
