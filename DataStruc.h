#include "stdafx.h"
#include <queue>

using namespace std;

#define ImageW 400
#define ImageH 400

enum Boundary {
	Top, Bottom, Left, Right
};

struct color {
	float r, g, b;		// Color (R,G,B values)
};

struct Point2D {
	float x, y;
};

struct PolygonStruc {
	queue<Point2D> q;
};

struct edge {
	float maxy;
	float currentx;
	float dx;
	edge *next;
};