#pragma once
#include <QtWidgets>


class H_Edge;

class Vertex {
	int index;
	double x, y, z;
	H_Edge* edge;
public:
	Vertex() {};
	Vertex(int i, double surX, double surY, double surZ) { index = i; x = surX; y = surY; z = surZ; };

	void set(int i, double surX, double surY, double surZ) { index = i; x = surX; y = surY; z = surZ; };
	void setEdge(H_Edge* E) { edge = E; };
	void setSur(int i, double surX, double surY, double surZ) { index = i; x = surX; y = surY; z = surZ; };
	void setX(double surX) { x = surX; };
	void setY(double surY) { y = surY; };
	void setZ(double surZ) { z = surZ; };

	QString getSur() { return QString::number(x) + " " + QString::number(y) + " " + QString::number(z) + " "; };
	int getIndex() { return index; };
	double getX() { return x; };
	double getY() { return y; };
	double getZ() { return z; };
};

class Face;

class H_Edge {
	Vertex* vert_origin;
	Vertex* vert_end;
	Face* face;
	H_Edge* edge_prev, * edge_next;
	H_Edge* pair;
public:
	H_Edge() { face = nullptr; };
	H_Edge(Vertex* vo, Face* f, H_Edge* ep, H_Edge* en, H_Edge* pr) { vert_origin = vo; face = f; edge_prev = ep; edge_next = en; pair = pr; };

	
	void set(Vertex* vo, Face* f, H_Edge* ep, H_Edge* en, H_Edge* pr = nullptr) { vert_origin = vo; face = f; edge_prev = ep; edge_next = en; pair = pr; };
	void setVert(Vertex* vo, Vertex* ve) { vert_origin = vo; vert_end = ve; };
	void setFace(Face* f) { face = f; };
	void setEdgePrev(H_Edge* ep) { edge_prev = ep; };
	void setEdgeNext(H_Edge* en) { edge_next = en; };
	void setPair(H_Edge* pr) { pair = pr; };

	QString PrintVrcholyHrany() { return QString::number(vert_origin->getIndex()) + " " + QString::number(edge_next->vert_origin->getIndex()) + " "; };
	QString PrintVrcholySteny() {return QString::number(vert_origin->getIndex()) + " " +QString::number(edge_next->vert_origin->getIndex()) + " " + QString::number(edge_prev->vert_origin->getIndex());
	};
	Face* getStena() { return face; };

	int getVOIndex() { return vert_origin->getIndex(); };
	int getVEIndex() { return vert_end->getIndex(); };
	H_Edge* getHrana_next() { return edge_next; };
	H_Edge* getHrana_prev() { return edge_prev; };
	H_Edge* getHrana_pair() { return pair; };
};

class Face {
	H_Edge* edge=nullptr;
public:
	Face() {};
	Face(H_Edge* E) { edge = E; };

	void setEdge(H_Edge* E) { edge = E; };
	H_Edge* getEdge() { return edge; };

	QString PrintVrcholy() { return edge->PrintVrcholySteny(); };
};



class Hedron {
	QList<Vertex>* Vrcholy=NULL;
	QList<H_Edge>* Hrany=NULL;
	QList<Face>* Steny=NULL;
public:
	void setVrcholy(QList<Vertex>* vrcholy) { Vrcholy = vrcholy; };
	void setHrany(QList<H_Edge>* hrany) { Hrany = hrany; };
	void setSteny(QList<Face>* steny) { Steny = steny; };
	void setParove() {};
	int getVrcholysize() { return Vrcholy->size(); };
	int getHranysize() { return Hrany->size(); };
	int getStenysize() { return Steny->size(); };
	int getHranaOIndex(int i) { return (*Hrany)[i].getVOIndex(); };
	int getPairHranaOIndex(int i) { return (*Hrany)[i].getHrana_pair()->getVOIndex(); };
	QString printSur(int i) { return (*Vrcholy)[i].getSur(); };
	QString printHrana(int i) { return (*Hrany)[i].PrintVrcholyHrany(); };
	QString printStena(int i) { return (*Steny)[i].PrintVrcholy(); };
	bool HisEmpty() { 
		if (Vrcholy == NULL && Hrany == NULL && Steny == NULL) return true;
		else return false;
	};
	void clear() { Vrcholy->clear(); Hrany->clear(); Steny->clear(); Vrcholy = NULL; Hrany = NULL; Steny = NULL; };
};
