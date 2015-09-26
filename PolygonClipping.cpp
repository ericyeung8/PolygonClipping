// PolygonClipping.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdlib.h>
#define GLUT_DISABLE_ATEXIT_HACK
#include <gl/glut.h>
#include <gl/gl.h>
#include <math.h>
#include "DataStruc.h"

/******************************************************************
	Notes:
	Image size is 400 by 400 by default.  You may adjust this if
		you want to.
	You can assume the window will NOT be resized.
	Call clearFramebuffer to clear the entire framebuffer.
	Call setFramebuffer to set a pixel.  This should be the only
		routine you use to set the color (other than clearing the
		entire framebuffer).  drawit() will cause the current
		framebuffer to be displayed.
	As is, your scan conversion should probably be called from
		within the display function.  There is a very short sample
		of code there now.
	You may add code to any of the subroutines here,  You probably
		want to leave the drawit, clearFramebuffer, and
		setFramebuffer commands alone, though.
  *****************************************************************/

/*Function Declaration*/
void PolygonFill(int num, struct PolygonStruc *pg);
int SutHodClip(Point2D rmin, Point2D rmax, int num, PolygonStruc *pout);

/*Global Variables*/
struct PolygonStruc plist[10];
float framebuffer[ImageH][ImageW][3];
int number = 0;
int c = 0, clip = 0;;
int flag = 0;
Point2D origin = {0 , 0};
Point2D mouse = {0, 0};

// Draws the scene
void drawit(void)
{
   glDrawPixels(ImageW,ImageH,GL_RGB,GL_FLOAT,framebuffer);
}

// Clears framebuffer to black
void clearFramebuffer()
{
	int i,j;

	for(i=0;i<ImageH;i++) {
		for (j=0;j<ImageW;j++) {
			framebuffer[i][j][0] = 0.0;
			framebuffer[i][j][1] = 0.0;
			framebuffer[i][j][2] = 0.0;
		}
	}
}

// Sets pixel x,y to the color RGB
// I've made a small change to this function to make the pixels match
// those returned by the glutMouseFunc exactly - Scott Schaefer 

void setFramebuffer(int x, int y, float R, float G, float B)
{
	// changes the origin from the lower-left corner to the upper-left corner
	y = ImageH - 1 - y;
	if (R<=1.0)
		if (R>=0.0)
			framebuffer[y][x][0]=R;
		else
			framebuffer[y][x][0]=0.0;
	else
		framebuffer[y][x][0]=1.0;
	if (G<=1.0)
		if (G>=0.0)
			framebuffer[y][x][1]=G;
		else
			framebuffer[y][x][1]=0.0;
	else
		framebuffer[y][x][1]=1.0;
	if (B<=1.0)
		if (B>=0.0)
			framebuffer[y][x][2]=B;
		else
			framebuffer[y][x][2]=0.0;
	else
		framebuffer[y][x][2]=1.0;
}

void display(void)
{
	Point2D rmin, rmax, temp;
	unsigned int i = 0, j = 0, k =0;
	float m;

	/*Draw the edge just input*/
	if (flag == 0 && plist[number].q.size() >= 2)
	{
		temp = plist[number].q.back();
		if (temp.x != origin.x)
			m = (temp.y - origin.y)/(temp.x - origin.x);
		if (temp.x <= origin.x)
		{
			j = (unsigned int)temp.x;
			k = (unsigned int)origin.x;
		}
		else
		{
			j = (unsigned int)origin.x;
			k = (unsigned int)temp.x;
		}
		for (i = j; i <= k; i++)
		{
			if (temp.x != origin.x)	//slope exists
				setFramebuffer(i, (int)(m*(i-temp.x)+temp.y),0.1*number, 0.1*(1-number), 0.1*number);
			else	//parallel to the y-axis
			{
				int l;
				if (temp.y <= origin.y)
					for (l = temp.y; l<= origin.y; l++)
						setFramebuffer(i, l,0.1*number, 0.1*(1-number), 0.1*number);
				else
					for (l = origin.y; l<= temp.y; l++)
						setFramebuffer(i, l,0.1*number, 0.1*(1-number), 0.1*number);
			}
		}
	}

	/*Calculate the rmin and rmax*/
	if (c && clip)
	{
		if (origin.x <= mouse.x)
		{
			rmin.x = origin.x;
			rmax.x = mouse.x;
		}
		else
		{
			rmin.x = mouse.x;
			rmax.x = origin.x;
		}

		if (origin.y <= mouse.y)
		{
			rmin.y = origin.y;
			rmax.y = mouse.y;
		}
		else
		{
			rmin.y = mouse.y;
			rmax.y = origin.y;
		}
	}

	/*Draw one polugon*/
	if (flag == 1)
	{
		struct PolygonStruc *pout;

		pout = new PolygonStruc;

		for (i=1; i <= plist[number-1].q.size(); i++)
		{
			temp = plist[number-1].q.front();
			pout->q.push(temp);
			plist[number-1].q.pop();
			plist[number-1].q.push(temp);
		}

		PolygonFill(number-1, pout);
	}

	/*Clip the polygon*/
	if (c && clip)
	{
		clearFramebuffer();
		glClear ( GL_COLOR_BUFFER_BIT );

		for (i = rmin.x; i <= rmax.x; i++)
		{
			setFramebuffer(i, rmin.y-1, 1, 0, 0);
			setFramebuffer(i, rmax.y+1, 1, 0, 0);
		}

		for (i = rmin.y; i <= rmax.y; i++)
		{
			setFramebuffer(rmin.x-1, i, 1, 0, 0);
			setFramebuffer(rmax.x+1, i, 1, 0, 0);
		}

		for (i=0; i<=(unsigned int)number-1; i++)
		{
			struct PolygonStruc *pg;
			pg = new PolygonStruc;
			SutHodClip(rmin, rmax, i, pg);
			delete pg;
		}
	}

	drawit();

	glFlush ( );
}

void init(void)
{
	gluOrtho2D ( 0, ImageW - 1, ImageH - 1, 0 );
	clearFramebuffer();
	glClear ( GL_COLOR_BUFFER_BIT );
}

void PolygonFill(int num, struct PolygonStruc *pg)
{
	unsigned int ymax = 0;
	unsigned int ymin = ImageH;
	unsigned int i = 0, j = 0;
	struct edge *ET[ImageH];
	struct edge *AEL;
	struct color rgb = {0.1*num, 0.1*(1-num), 0.1*num};
	Point2D temp;
	Point2D temp1, temp2;

	AEL = new edge;
	AEL->next = NULL;

	/*Initialize the active edge table*/
	for (i = 0; i <= ImageH-1; i++)
	{
		ET[i] = new edge;
		ET[i]->next = NULL;
	}

	/*Calculate the ymax and ymin of the polygon*/
	for (i=1; i<= pg->q.size(); i++)
	{
		temp = pg->q.front();
		pg->q.pop();
		if (ymax <= temp.y)
			ymax = temp.y;
		if (ymin >= temp.y)
			ymin = temp.y;
		pg->q.push(temp);
	}

	/*Insert the edge to the active edge table*/
	for (i=ymin; i <= ymax; i++)
	{
		for (j=1; j <= pg->q.size(); j++)
		{
			temp1 = pg->q.front();
			pg->q.pop();

			if ((int)temp1.y == i)
			{
				temp2 = pg->q.front();

				if ((int)temp1.y > (int)temp2.y)
				{
					edge *p = new edge;
					edge *q = new edge;

					q = ET[(int)temp2.y];
					while (q->next != NULL)	//find the last element
						q = q->next;

					p->maxy = temp1.y;
					p->currentx = temp2.x;
					p->dx = (temp1.x - temp2.x)/(temp1.y - temp2.y);
					p->next = NULL;
					q->next = p;
				}
				else if ((int)temp1.y < (int)temp2.y)
				{
					edge *p = new edge;
					edge *q = new edge;

					q = ET[(int)temp1.y];
					while (q->next != NULL)
						q = q->next;

					p->maxy = temp2.y;
					p->currentx = temp1.x;
					p->dx = (temp1.x - temp2.x)/(temp1.y - temp2.y);
					p->next = NULL;
					q->next = p;
				}
			}
			pg->q.push(temp1);	//push the first point into queue
		}
	}

	/*Draw the polygon*/
	for (i = ymin; i <= ymax; i++)
	{
		edge *p = new edge;
		edge *q = new edge;

		p = AEL->next;

		while (p) //calculate the new x position
		{
			p->currentx = p->currentx + p->dx;
			p = p->next;
		}

		/*sort the linked list, to be continued*/
		edge *r = new edge;
		p = AEL;
		while (p->next && p->next->next)	//at least 2 elements
		{
			q = p->next;
			r = q->next;
			while (r)
			{
				edge *s = new edge;
				if (q->currentx > r->currentx)
				{
					p->next = r;
					q->next = r->next;
					r->next = q;
					s = r;
					r = q->next;
					q = s;
				}
				else	r = r->next;
			}
			p = q;
		}

		r = AEL;
		p = ET[i]->next;
		q = AEL->next;
		while (p)	//+edge
		{
			if (q != NULL)
			{
				if (p->currentx <= (int)q->currentx)	//may have error due to the dx
				{
					edge *s = new edge;
					s->currentx = p->currentx;
					s->dx = p->dx;
					s->maxy = p->maxy;
					s->next = q;
					r->next = s;
					p = p->next;

					r = AEL;
					q = AEL->next;
				}
				else
				{
					r = r->next;
					q = q->next;
				}
			}
			else
			{
					edge *s = new edge;
					s->currentx = p->currentx;
					s->dx = p->dx;
					s->maxy = p->maxy;
					s->next = NULL;
					r->next = s;
					p = p->next;
					r = AEL;
					q = AEL->next;
			}

		}

		p = AEL;
		q = AEL->next;
		while (q)	//delete edge
		{
			if ((int)q->maxy == i)
			{
				p->next = q->next;
				delete q;
				q = p->next;
			}
			else
			{
				q = q->next;
				p = p->next;
			}
		}

		p = AEL->next;
		while (p && p->next)	//setframebuffer
		{
			q = p->next;
			for (j = ceil(p->currentx); j <= (unsigned int)q->currentx; j++)
				setFramebuffer((int)j, (int)i, rgb.r, rgb.g, rgb.b);
			p = q->next;
		}
	}
}

/*classify whether point p is inside the boundary b*/
int inside(Point2D p, Boundary b, Point2D rmin, Point2D rmax)
{
	switch (b)
	{
	case Top:
		if (p.y > rmax.y)	return 0;
		break;
	case Bottom:
		if (p.y < rmin.y)	return 0;
		break;
	case Left:
		if (p.x < rmin.x)	return 0;
		break;
	case Right:
		if (p.x > rmax.x)	return 0;
		break;
	}
	return 1;
}

/*classify whether point p1 and point p2 are crossing the boundary b*/
int cross(Point2D p1, Point2D p2, Boundary b, Point2D rmin, Point2D rmax)
{
	if (inside(p1, b, rmin, rmax) == inside(p2, b, rmin, rmax))
		return 0;
	else return 1;
}

/*Calculate the intersectpoint*/
Point2D IntersectPoint(Point2D p1, Point2D p2, Boundary b, Point2D rmin, Point2D rmax)
{
	Point2D p;
	float a;

	if (p1.x != p2.x)
		a = (p1.y - p2.y)/(p1.x - p2.x);

	switch (b)
	{
	case Top:
		p.y = rmax.y;
		if (p1.x != p2.x)
			p.x = (p.y - p2.y)/a + p2.x;
		else p.x = p1.x;
		break;
	case Bottom:
		p.y = rmin.y;
		if (p1.x != p2.x)
			p.x = (p.y - p2.y)/a + p2.x;
		else p.x = p1.x;
		break;
	case Left:
		p.x = rmin.x;
		p.y = a*(p.x - p2.x) + p2.y;
		break;
	case Right:
		p.x = rmax.x;
		p.y = a*(p.x - p2.x) + p2.y;
		break;
	}

	return p;
}

/*Clip the polygon*/
int SutHodClip(Point2D rmin, Point2D rmax, int num, PolygonStruc *pout)
{
	unsigned int i = 0;
	Point2D temp, temp1, temp2;
	Point2D IPt;
	Boundary b = Top;

	/*Initialize the queue pout*/
	for (i=1; i <= plist[num].q.size(); i++)
	{
		temp = plist[num].q.front();
		pout->q.push(temp);
		plist[num].q.pop();
		plist[num].q.push(temp);
	}
	temp = plist[num].q.front();
	pout->q.push(temp);

	/*Clip the polygon*/
	for (b = Top; b <= Right; b = Boundary(b+1))
	{
		int cnt;
		cnt = pout->q.size();

		for (i=1; i <= (unsigned int)cnt-1; i++)
		{
			temp1 = pout->q.front();
			pout->q.pop();
			temp2 = pout->q.front();
			if (cross(temp1, temp2, b, rmin, rmax))
			{
				IPt = IntersectPoint(temp1, temp2, b, rmin, rmax);
				if (!inside(temp1, b, rmin, rmax))
				{
					pout->q.push(IPt);
					pout->q.push(temp2);
				}
				else
					pout->q.push(IPt);
			}
			else if (inside(temp1, b, rmin, rmax))
				pout->q.push(temp2);
		}

		pout->q.pop();
		if (pout->q.size() == 0)	//in case no output point
			return 0;
		else
		{
			temp = pout->q.front();
			pout->q.push(temp);
		}
	}

	pout->q.pop();
	PolygonFill(num, pout);

	return 1;
}

void mouseClick(int button, int state, int x, int y)
{
	Point2D current;
	current.x = x;
	current.y = y;

	if (!c)	//before clip
	{
		switch (button)
		{
		case GLUT_LEFT_BUTTON:
			if (state == GLUT_DOWN)
			{
				if (plist[number].q.size() != 0)	//in order to draw the line immediately
					origin = plist[number].q.back();
				plist[number].q.push(current);
				flag = 0;
			}
			break;
		case GLUT_RIGHT_BUTTON:
			flag = 1;
			if (state == GLUT_DOWN)
			{
				plist[number].q.push(current);
				number ++;
			}
			break;
		}
	}
	else	//store the origin point of clipping rectangle
	{
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && !clip)
		{
			origin.x = x;
			origin.y = y;
		}
		else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP && clip)	//reclip
			clip = 0;
	}

	glutPostRedisplay();
}

void MouseMove(int x, int y)
{
	if (c)	//store the temp point
	{
		mouse.x = x;
		mouse.y = y;
		clip = 1;
		glutPostRedisplay();
	}
}

void keyboard(unsigned char key, int x, int y)
{
	if (key == 'c')
	{
		c = 1;
		flag = 0;
	}
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB);
	glutInitWindowSize(ImageW,ImageH);
	glutInitWindowPosition(400,400);
	glutCreateWindow("Jiajun Yang - Homework 2");
	init();	
	glutDisplayFunc(display);
	glutMouseFunc(mouseClick);
	glutMotionFunc(MouseMove);
	glutKeyboardFunc(keyboard);
	glutMainLoop();
	return 0;
}