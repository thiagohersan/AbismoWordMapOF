#pragma once

#include "ofConstants.h"
#include "ofRectangle.h"
#include "ofTrueTypeFont.h"
#include "ofEvents.h"
#include<map>
#include<queue>
#include<set>

class Edge;
class Node;

class PhysNode {
	public:
		PhysNode(const string name_);
		~PhysNode();
		void setVelocity(const ofVec2f& vel_);
		void setSize(const float size_);
		void setPos(const ofVec2f& pos_);
		const string getName() const;
		const ofVec2f& getPos() const;
		const float& getSize() const;
		const ofRectangle& getBoundingBox() const;
		inline const bool isMouseInside(ofMouseEventArgs & args) const;
		void draw();
	protected:
		ofVec2f pos, vel;
		float size;
		string name;
		ofRectangle boundingBox;
		// static font map
		static map<int, ofTrueTypeFont> fontMap;
};

class Node: public PhysNode {
	public:
		Node(const string name_);
		~Node();
		void setDistance(float f);
		const float getDistance() const;
		const map<string, Edge*>& getEdges() const;
		const bool isInQ() const;
		void setInQ(const bool q);
		void process() const;
		void addEdge(Edge* e);
		static ofEvent<Node> addNodeToGraph;
		bool operator < (const Node &on) const;
		static bool sortComp(Node *n0, Node *n1);
		// click events for re-sorting
		ofEvent<Node> NodeClickEvent;
		void mouseMoved(ofMouseEventArgs & args);
		void mouseDragged(ofMouseEventArgs & args);
		void mousePressed(ofMouseEventArgs & args);
		void mouseReleased(ofMouseEventArgs & args);
	protected:
		float distance;
		map<string, Edge*> theEdges;
		bool bInQ;
};

class Edge: public PhysNode {
	public:
		Edge(const string name_, const int cost_);
		~Edge();
		void setCost(const float td);
		void resetMinCost();
		float getCost() const;
		const map<string, Node*>& getNodes() const;
		void addNode(Node* n);
		static ofEvent<Node> addNodeToQ;
		static ofEvent<Edge> addEdgeToGraph;
		bool operator < (const Edge &oe) const;
		static bool sortComp(Edge *e0, Edge *e1);
		// click events for triggering sub-menu
		ofEvent<Edge> EdgeClickEvent;
		void mouseMoved(ofMouseEventArgs & args);
		void mouseDragged(ofMouseEventArgs & args);
		void mousePressed(ofMouseEventArgs & args);
		void mouseReleased(ofMouseEventArgs & args);
	protected:
		float minCost, cost, avgCost;
		map<string,Node*> theNodes;
};

class Graph {
	public:
		Graph();
		~Graph();
		void addNodeToGraph(Node& n);
		void addEdgeToGraph(Edge& e);
		void addNodeToQ(Node& n);
		void calculateDists(Node& fromNode);
		void orderGraph();
		// physical
		void nodeClickListener(Node& n);
		void edgeClickListener(Edge& e);
		void update();
		void draw();
		// debug
		void printGraph() const;
		void calculateDists();
	protected:
		map<string, Node*> theNodes;
		map<string, Edge*> theEdges;
		vector<Node*> orderedNodes;
		vector<Edge*> orderedEdges;
		queue<Node*> theQ;
		// for drawing Graph
		ofRectangle drawArea;
};
