#include "OpenGLDemo.h"
#include <time.h>
#include <math.h>
// GLUT header
#include <stdlib.h>
#include <OpenGL\glut.h>    // OpenGL GLUT Library Header
// Open file dialog
#include "LoadFileDlg.h"
// The GLM code for loading and displying OBJ mesh file
#include "glm.h"
// The trackball code for rotating the model
#include "trackball.h"
#define BITMAP_ID 0x4D42
#define PI 3.1415926
// The size of the GLUT window
int window_width  = 800;
int window_height = 600;

// The OBJ model
GLMmodel* pModel = NULL;

// The current modelview matrix
double pModelViewMatrix[16];

// If mouse left button is pressed
bool bLeftBntDown = false;

// Old position of the mouse
int OldX = 0;
int OldY = 0;

//WriteBitmapFile
//根据bitmapData的（RGB）数据，保存bitmap
//filename是要保存到物理硬盘的文件名（包括路径）
BOOL WriteBitmapFile(char * filename, int width, int height, unsigned char * bitmapData)
{
	//填充BITMAPFILEHEADER
	BITMAPFILEHEADER bitmapFileHeader;
	memset(&bitmapFileHeader, 0, sizeof(BITMAPFILEHEADER));
	bitmapFileHeader.bfSize = sizeof(BITMAPFILEHEADER);
	bitmapFileHeader.bfType = 0x4d42;	//BM
	bitmapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);

	//填充BITMAPINFOHEADER
	BITMAPINFOHEADER bitmapInfoHeader;
	memset(&bitmapInfoHeader, 0, sizeof(BITMAPINFOHEADER));
	bitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfoHeader.biWidth = width;
	bitmapInfoHeader.biHeight = height;
	bitmapInfoHeader.biPlanes = 1;
	bitmapInfoHeader.biBitCount = 24;
	bitmapInfoHeader.biCompression = BI_RGB;
	bitmapInfoHeader.biSizeImage = width * abs(height) * 3;

	//////////////////////////////////////////////////////////////////////////
	FILE * filePtr;			//连接要保存的bitmap文件用
	unsigned char tempRGB;	//临时色素
	int imageIdx;

	//交换R、B的像素位置,bitmap的文件放置的是BGR,内存的是RGB
	for (imageIdx = 0; imageIdx < bitmapInfoHeader.biSizeImage; imageIdx += 3)
	{
		tempRGB = bitmapData[imageIdx];
		bitmapData[imageIdx] = bitmapData[imageIdx + 2];
		bitmapData[imageIdx + 2] = tempRGB;
	}

	filePtr = fopen(filename, "wb");
	if (NULL == filePtr)
	{
		return FALSE;
	}

	fwrite(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);

	fwrite(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);

	fwrite(bitmapData, bitmapInfoHeader.biSizeImage, 1, filePtr);

	fclose(filePtr);
	return TRUE;
}
//SaveScreenShot
//保存窗口客户端的截图
//窗口大小* 600
void SaveScreenShot()
{
	int clnWidth, clnHeight;	//client width and height
	static void * screenData;
	RECT rc;
	int len = 800 * 600 * 3;
	screenData = malloc(len);
	memset(screenData, 0, len);
	glReadPixels(0, 0, 800, 600, GL_RGB, GL_UNSIGNED_BYTE, screenData);
	//生成文件名字符串，以时间命名
	time_t tm = 0;
	tm = time(NULL);
	char lpstrFilename[256] = { 0 };
	sprintf_s(lpstrFilename, sizeof(lpstrFilename), "%d.bmp", tm);
	WriteBitmapFile(lpstrFilename, 800, 600, (unsigned char*)screenData);
	//释放内存
	free(screenData);
}


int main(int argc, char* argv[])
{
	// Initialize the GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);
	//glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(window_width, window_height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("OpenGL Demo");
	init();

	// Set the callback function
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);
	
	glutMainLoop();
	
	return 0;
}

/// Initialize the OpenGL
void init()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(atan(tan(30.0/180*PI / 2)*window_height / window_width)*2/PI*180, (float)window_width / (float)window_height, 1.0f, 400.0f);//atan(tan(fov/2)*window_height/window_width)*2
	//gluPerspective(30.0f, (float)window_width / (float)window_height, 0.1f, 4.5f);
	GLfloat m[16];
	glGetFloatv(GL_PROJECTION_MATRIX, m);
	glClearColor(0, 0, 0, 1);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	//glCullFace(GL_BACK);
	//glEnable(GL_CULL_FACE);
    
    //Setup other misc features.
    glEnable( GL_LIGHTING );
    glEnable( GL_NORMALIZE );
    glShadeModel( GL_SMOOTH );
    
    // Setup lighting model.
	GLfloat light_model_ambient[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat light0_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat light0_direction[] = {0.0f, 0.0f, 10.0f, 0.0f};
	GLfloat light_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};

	glLightfv(GL_LIGHT0, GL_POSITION, light0_direction);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_model_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glEnable(GL_LIGHT0);

	// Init the dlg of the open file
	PopFileInitialize( NULL );
}

/// Display the Object
void display()
{
	GLfloat m[16];
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslated(0.0, 0.0, 0.0);
	glMultMatrixd(pModelViewMatrix);
	glGetFloatv(GL_MODELVIEW_MATRIX, m);
	//gluLookAt(30.0, 30.0, 200.0, 0.0, 0.0, 0.0, -0.7147, -0.6839, 0.1467);
	glGetFloatv(GL_MODELVIEW_MATRIX, m);
	GLdouble equn1[4] = { 0.0f, 0.0f, -1.0f, 20.0 };
	GLint viewport[16];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glClipPlane(GL_CLIP_PLANE1, equn1);
//	glEnable(GL_CLIP_PLANE1);

	if (pModel)
	{
		glmDraw(pModel, GLM_FLAT);
	}

	glColor3f(1.0f, 0, 0);
	glTranslatef(0, 0, 20.0f);
	glutSolidSphere(0.3, 16, 16);
	/*glTranslatef(0, -60.0f, 0.0f);
	glutSolidSphere(0.3, 16, 16);
	glTranslatef(-60.0, 0.0f, 0.0f);
	glutSolidSphere(0.3, 16, 16);
	glTranslatef(0, 60.0f, 0.0f);
	glutSolidSphere(0.3, 16, 16);*/
	glutSwapBuffers();
	glFlush();
}

/// Reshape the Window
void reshape(int w, int h)
{
	// Update the window's width and height
	window_width  = w;
	window_height = h;

	// Reset the viewport
	glViewport(0, 0, window_width, window_height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(atan(tan(30.0 / 180 * PI / 2)*window_height / window_width) * 2 / PI * 180, (float)window_width / (float)window_height, 1.0f, 400.0f);
	glutPostRedisplay();
}

/// Keyboard Messenge
void keyboard(unsigned char key, int x, int y)
{
	// The obj file will be loaded
	char FileName[128] = "Model\\cow.obj";
	char TitleName[128]= "Model\\cow.obj";

	// Center of the model
	float modelCenter[] = {0.0f, 0.0f, 0.0f};

	switch ( key )
	{
	case 'o':
	case 'O':
		PopFileOpenDlg( NULL, FileName, TitleName );

		// If there is a obj model has been loaded, destroy it
		if ( pModel )
		{
			glmDelete( pModel );
			pModel = NULL;
		}

		// Load the new obj model
		pModel = glmReadOBJ( FileName );
		
		// Generate normal for the model
		glmFacetNormals( pModel );

		// Scale the model to fit the screen
		//glmUnitize( pModel, modelCenter );

		// Init the modelview matrix as an identity matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glGetDoublev( GL_MODELVIEW_MATRIX, pModelViewMatrix );	

		break;

	case '+':
		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();
		glLoadMatrixd( pModelViewMatrix );
		glScaled( 1.05, 1.05, 1.05 );
		glGetDoublev( GL_MODELVIEW_MATRIX, pModelViewMatrix );
		break;

	case '-':
		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();
		glLoadMatrixd( pModelViewMatrix );
		glScaled( 0.95, 0.95, 0.95 );
		glGetDoublev( GL_MODELVIEW_MATRIX, pModelViewMatrix );
		break;
	case 's':
		glutPostRedisplay();
		SaveScreenShot();
		break;
	case 'd':
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-50, 50, -50, 50, -50, 50);
		GLfloat m[16];
		glGetFloatv(GL_PROJECTION_MATRIX, m);
		glutPostRedisplay();
		break;
	default:
		break;
	}

	glutPostRedisplay();
}

/// Mouse Messenge
void mouse(int button, int state, int x, int y)
{
	if ( pModel )
	{
		if ( state==GLUT_DOWN && button==GLUT_LEFT_BUTTON )
		{
			OldX = x;
			OldY = y;
			bLeftBntDown = true;
		}
		else if ( state==GLUT_DOWN && button==GLUT_LEFT_BUTTON )
		{
			bLeftBntDown = false;
		}
	}
}

/// Motion Function
void motion(int x, int y)
{
	if ( bLeftBntDown && pModel )
	{
		float fOldX =  2.0f*OldX/(float)window_width  - 1.0f;
		float fOldY = -2.0f*OldY/(float)window_height + 1.0f;
		float fNewX =  2.0f*x/(float)window_width  - 1.0f;
		float fNewY = -2.0f*y/(float)window_height + 1.0f;

		double pMatrix[16];
		trackball_opengl_matrix( pMatrix, fOldX, fOldY, fNewX, fNewY);

		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();
		glLoadMatrixd( pMatrix );
		glMultMatrixd( pModelViewMatrix );
		glGetDoublev( GL_MODELVIEW_MATRIX, pModelViewMatrix );

		OldX = x;
		OldY = y;
		glutPostRedisplay();
	}
}

/// Idle function
void idle(void)
{
	
	glutPostRedisplay();
}