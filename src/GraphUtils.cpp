#include "GraphUtils.h"
#include "ofAppRunner.h"
#include "ofGraphics.h"
#include "ofLog.h"

#define MAX_FONT_SIZE 64
#define MIN_FONT_SIZE 16
#define FONT_NAME "verdana.ttf"

map<int,ofTrueTypeFont> PhysNode::fontMap = map<int,ofTrueTypeFont>();

PhysNode::PhysNode(const string name_){
	name = name_;
	setPos(ofVec2f(ofRandom(ofGetWidth()),ofRandom(ofGetHeight())));
	vel = ofVec2f(0,0);
	if(fontMap.size() < 1){
		for(int i=0; i<11; ++i){
			int fs = i*5+MIN_FONT_SIZE;
			ofTrueTypeFont ottf;
			ottf.loadFont(FONT_NAME, fs, true, true);
			fontMap[fs] = ottf;
		}
	}
	setSize(20);
}
PhysNode::~PhysNode(){}

void PhysNode::setVelocity(const ofVec2f& vel_){
	vel = vel_;
}
void PhysNode::setSize(const float size_){
	size = size_;
	std::map<int,ofTrueTypeFont>::iterator it = fontMap.lower_bound(size/2);
	// keep it in-bounds
	(it == fontMap.end())?(--it):(it);
	ofTrueTypeFont mFont = (it->second);
	// set size of bounding box
	boundingBox.width = mFont.stringWidth(name);
	boundingBox.height = mFont.stringHeight(name);
}

void PhysNode::setPos(const ofVec2f& pos_){
	pos = pos_;
	boundingBox.x = pos.x;
	boundingBox.y = pos.y;
}

const string PhysNode::getName() const{
	return name;
}
const ofVec2f& PhysNode::getPos() const{
	return pos;
}
const float& PhysNode::getSize() const{
	return size;
}

const ofRectangle& PhysNode::getBoundingBox() const{
	return boundingBox;
}

void PhysNode::draw(){
	std::map<int,ofTrueTypeFont>::iterator it = fontMap.lower_bound(size/2);
	// keep it in-bounds
	(it == fontMap.end())?(--it):(it);
	ofTrueTypeFont mFont = (it->second);
	ofSetColor(100,100);
	ofRect(boundingBox);
	ofSetColor(255);
	// magic to account for the fact that rectangles draw from top-left and strings from bottom-left corner
	ofRectangle tbb = mFont.getStringBoundingBox(name, pos.x, pos.y);
	mFont.drawString(name, pos.x+(pos.x-tbb.x), pos.y+(pos.y-tbb.y));
}

inline const bool PhysNode::isMouseInside(ofMouseEventArgs & args) const {
	return boundingBox.inside(args.x, args.y);
}

//////////////////////////////////
//////////////////////////////////
//////////////////////////////////
ofEvent<Node> Node::addNodeToGraph = ofEvent<Node>();

Node::Node(const string name_): PhysNode(name_){
	distance = 1e9;
	ofNotifyEvent(Node::addNodeToGraph, *this);
	ofRegisterMouseEvents(this);
}

Node::~Node(){
	ofUnregisterMouseEvents(this);
}

bool Node::operator < (const Node &on) const{
	return distance < on.distance;
}

bool Node::sortComp(Node *n0, Node *n1){
	return *n0 < *n1;
}

void Node::setDistance(float f){
	distance = f;
}
const float Node::getDistance() const{
	return distance;
}
const bool Node::isInQ() const{
	return bInQ;
}
void Node::setInQ(const bool q){
	bInQ = q;
}

const map<string, Edge*>& Node::getEdges() const{
	return theEdges;
}

void Node::process() const{
	// send my distance to all edges
	for (map<string,Edge*>::const_iterator it=theEdges.begin(); it!=theEdges.end(); ++it){
		(it->second)->setCost(distance);
	}
}

// add edge to map. use name as key.
void Node::addEdge(Edge* e){
	if(theEdges.find(e->getName()) == theEdges.end()){
		theEdges[e->getName()] = e;
		// link back to node...
		e->addNode(this);
	}
}

void Node::mouseMoved(ofMouseEventArgs & args){}
void Node::mouseDragged(ofMouseEventArgs & args){}
void Node::mousePressed(ofMouseEventArgs & args){
	if(this->isMouseInside(args)){
		// DEBUG
		setSize(size+10);
		ofNotifyEvent(NodeClickEvent, *this);
	}
}
void Node::mouseReleased(ofMouseEventArgs & args){}

//////////////////////////////////
//////////////////////////////////
//////////////////////////////////
ofEvent<Node> Edge::addNodeToQ = ofEvent<Node>();
ofEvent<Edge> Edge::addEdgeToGraph = ofEvent<Edge>();

Edge::Edge(const string name_, const int cost_): PhysNode(name_){
	cost = cost_;
	minCost = 1e9;
	ofNotifyEvent(Edge::addEdgeToGraph, *this);
	ofRegisterMouseEvents(this);
}
Edge::~Edge(){
	ofUnregisterMouseEvents(this);
}

bool Edge::operator < (const Edge &oe) const{
	return minCost < oe.minCost;
}

bool Edge::sortComp(Edge *e0, Edge *e1){
	return *e0 < *e1;
}

void Edge::setCost(const float td){
	// if there's a shorter way to get here, update minCost
	if(td+avgCost < minCost){
		minCost = td+avgCost;
		
		// send new cost to all nodes
		for (map<string,Node*>::const_iterator it=theNodes.begin(); it!=theNodes.end(); ++it){
			if(minCost+avgCost < (it->second)->getDistance()){
				(it->second)->setDistance(minCost+avgCost);
				// Add node to graph Q
				(it->second)->setInQ(true);
				ofNotifyEvent(Edge::addNodeToQ, *(it->second));
			}
		}
	}
}

void Edge::resetMinCost(){
	minCost = 1e9;
}

float Edge::getCost() const{
	return minCost;
}

const map<string, Node*>& Edge::getNodes() const{
	return theNodes;
}

void Edge::addNode(Node* n){
	if(theNodes.find(n->getName()) == theNodes.end()){
		theNodes[n->getName()] = n;
		avgCost = cost/theNodes.size();
	}
}

// click events for triggering sub-menu
void Edge::mouseMoved(ofMouseEventArgs & args){}
void Edge::mouseDragged(ofMouseEventArgs & args){}
void Edge::mousePressed(ofMouseEventArgs & args){
	if(this->isMouseInside(args)){
		// DEBUG
		setSize(size+10);
		ofNotifyEvent(EdgeClickEvent, *this);
	}
}
void Edge::mouseReleased(ofMouseEventArgs & args){}

//////////////////////////////////////
//////////////////////////////////////
//////////////////////////////////////

// adds every created node and edge to graph using listeners. ????
Graph::Graph(){
	ofAddListener(Edge::addNodeToQ, this, &Graph::addNodeToQ);
	ofAddListener(Node::addNodeToGraph, this, &Graph::addNodeToGraph);
	ofAddListener(Edge::addEdgeToGraph, this, &Graph::addEdgeToGraph);
	// draw area. hard coded for now
	drawArea = ofRectangle(10,50,700,500);
}

Graph::~Graph(){
	for (map<string,Node*>::const_iterator it=theNodes.begin(); it!=theNodes.end(); ++it){
		delete it->second;
	}
	for (map<string,Edge*>::const_iterator it=theEdges.begin(); it!=theEdges.end(); ++it){
		delete it->second;
	}
}

void Graph::addNodeToGraph(Node& n){
	theNodes[n.getName()] = &n;
	orderedNodes.push_back(&n);
	ofAddListener(n.NodeClickEvent, this, &Graph::nodeClickListener);
}
void Graph::addEdgeToGraph(Edge& e){
	theEdges[e.getName()] = &e;
	orderedEdges.push_back(&e);
	ofAddListener(e.EdgeClickEvent, this, &Graph::edgeClickListener);
}

void Graph::addNodeToQ(Node& n){
	// add to Q
	theQ.push(&n);
}

void Graph::calculateDists(Node& fromNode){
	// clear costs from nodes
	for (map<string,Node*>::const_iterator it=theNodes.begin(); it!=theNodes.end(); ++it){
		(it->second)->setDistance(1e9);
	}
	// clear minDist from edges
	for (map<string,Edge*>::const_iterator it=theEdges.begin(); it!=theEdges.end(); ++it){
		(it->second)->resetMinCost();
	}
	
	// push root calculate dists.
	fromNode.setDistance(0);
	theQ.push(&fromNode);
	
	while(!theQ.empty()){
		Node n = *(theQ.front());
		n.setInQ(false);
		theQ.pop();
		n.process();
	}
}

void Graph::orderGraph(){
	// check sizes
	if(orderedNodes.size() != theNodes.size()){
		ofLogWarning("orderedNodes and theNodes are inconsistent");
		orderedNodes.clear();
		for (map<string,Node*>::const_iterator it=theNodes.begin(); it!=theNodes.end(); ++it){
			orderedNodes.push_back(it->second);
		}
	}
	if(orderedEdges.size() != theEdges.size()){
		ofLogWarning("orderedEdges and theEdges are inconsistent");
		orderedEdges.clear();
		for (map<string,Edge*>::const_iterator it=theEdges.begin(); it!=theEdges.end(); ++it){
			orderedEdges.push_back(it->second);
		}
	}
	// sort
	sort(orderedNodes.begin(), orderedNodes.end(), Node::sortComp);
	sort(orderedEdges.begin(), orderedEdges.end(), Edge::sortComp);
}

// physical functions
void Graph::nodeClickListener(Node& n) {
	calculateDists(n);
	// TODO: update PhysNode sizes after calculating distances
	// update graph
	update();
}
void Graph::edgeClickListener(Edge& e) {
	// TODO: what to do when edge is clicked? open a sub menu?
	// update graph
	update();
}

void Graph::update(){
	float maxX, maxY, lineY, cX, cY;
	maxX = maxY = lineY = cX = cY = 0;
	bool reDoLine = true;
	map<string,Edge*>::const_iterator firstInLine=theEdges.begin();

	// TODO: mix edges and nodes somehow
	for (map<string,Edge*>::const_iterator it=theEdges.begin(); it!=theEdges.end(); ){
		Edge* e = it->second;
		if((e) && (e->getSize() > 1)) {
			// check cy to see if we can tuck this word here
			if(cY+e->getBoundingBox().height > maxY-5) {
				cY = lineY;
				cX = maxX + 5;
			}
			// have valid cY here

			// if next word is bigger than space available on this line
			if(cX+e->getBoundingBox().width > drawArea.width){
				// place each line 2 times
				if(reDoLine) {
					it = firstInLine;
					cX = 0;
					maxX = 0;
					reDoLine = false;
					continue;
				}
				// if we've already done this line twice, go to next line
				else {
					cY = maxY;
					lineY = maxY;
					cX = 0;
					maxX = 0;
					firstInLine = it;
					reDoLine = true;
				}
			}

			// have a valid (cX,cY) here
			e->setPos(ofVec2f(drawArea.x+cX,drawArea.y+cY));

			// update max values seen
			if(cX+e->getBoundingBox().width > maxX){
				maxX = cX+e->getBoundingBox().width;
			}
			if(cY+e->getBoundingBox().height > maxY){
				maxY = cY+e->getBoundingBox().height;
			}

			// finally update cX,cY
			cY += e->getBoundingBox().height;
		}
		// hack to keep it from incrementing when we re-do a line
		++it;
	}
	for (map<string,Node*>::const_iterator it=theNodes.begin(); it!=theNodes.end(); ++it){
		Node* n = it->second;
		if(n){
			//n->draw();
		}
	}
}

void Graph::draw(){
	// draw Edges
	for (map<string,Edge*>::const_iterator it=theEdges.begin(); it!=theEdges.end(); ++it){
		Edge* e = it->second;
		if(e) {
			e->draw();
		}
	}
	// draw Nodes
	for (map<string,Node*>::const_iterator it=theNodes.begin(); it!=theNodes.end(); ++it){
		Node* n = it->second;
		if(n){
			//n->draw();
		}
	}
}

// for DEBUG
void Graph::calculateDists(){
	int mi = (int)ofRandom(theNodes.size());
	int i = 0;
	Node *n0 = NULL;
	for (map<string,Node*>::const_iterator it=theNodes.begin(); it!=theNodes.end()&&i<mi; ++it,++i){
		n0 = it->second;
	}
	if(n0){
		long long unsigned int t0 = AbsoluteToDuration(UpTime());
		calculateDists(*n0);
		long long unsigned int et = AbsoluteToDuration(UpTime())-t0;
		cout << "calculated from: " << n0->getName()+ " in: " << et << " millis"<<endl;
	}
}

// for DEBUG
void Graph::printGraph() const{
	ofLogWarning("Nodes:")<< "";
	for (map<string,Node*>::const_iterator it=theNodes.begin(); it!=theNodes.end(); ++it){
		ofLogWarning() << it->first << ": " << (it->second)->getDistance();
	}
	ofLogWarning("Edges:") << "";
	for (map<string,Edge*>::const_iterator it=theEdges.begin(); it!=theEdges.end(); ++it){
		ofLogWarning() << it->first << ": " << (it->second)->getCost();
	}
}
