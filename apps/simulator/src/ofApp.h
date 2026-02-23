#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "lightpath/legacy.hpp"
#define OSC_PORT 54321
#define MAX_BRIGHTNESS 255

glm::vec2 pointOnEllipse(float rad, float w, float h);

class ofApp : public ofBaseApp{

public:
    enum ObjectType {
        OBJ_HEPTAGON919 = 0,
        OBJ_LINE = 1,
        OBJ_CROSS = 2,
        OBJ_TRIANGLE = 3,
        OBJ_HEPTAGON3024 = 4
    };
    
    lightpath::Object *object;
    lightpath::RuntimeState *state;
    lightpath::Debugger *debugger;
    ObjectType currentObjectType;

    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    void updateOsc();
    void onCommand(const ofxOscMessage& m);
    void onEmit(const ofxOscMessage& m);
    void onNoteOn(const ofxOscMessage& m);
    void onNoteOff(const ofxOscMessage& m);
    void onNotesSet(const ofxOscMessage& m);
    void onAuto(const ofxOscMessage& m);
    void parseParams(lightpath::EmitParams &p, const ofxOscMessage &m);
    void parseParam(lightpath::EmitParams &p, const ofxOscMessage &m, lightpath::EmitParam &param, uint8_t j);
    void doCommand(char command);
    glm::vec2 intersectionPos(lightpath::Intersection* intersection, int8_t j = -1);
    lightpath::Object* createObject(ObjectType type, uint16_t pixelCount);
    ofColor getColor(uint16_t i);
    void doEmit(lightpath::EmitParams &params);

    ofxOscReceiver receiver;
    uint8_t showModel = 0;
    bool showAll = false;
    bool showFps = false;
    bool showHeap = false;
    bool showPixels = false;
    int8_t lastList = -1;

};
