//
//  ofxLabelManager.hpp
//  Orbits
//
//  Created by Patricio González Vivo on 6/27/18.
//

#pragma once

#include "ofMain.h"
#include "ofxBody.h"

#define MARGIN 25.0
#define CHAR_WIDTH 8.0
#define CHAR_HEIGHT 10.0
//#define CHECK_STRAIGHT_FIRST
#define OCCLUTION_LENGTH 10.0

struct ofxLabel {
    string      text;
    glm::vec3   world_position;
    glm::vec3   screen_center;
    glm::vec3   screen_position;
    glm::vec3   screen_proj1;
    glm::vec3   screen_proj2;
    float       width;
    bool        bLeft;
    bool        bTop;
    bool        bVisible;
};

class ofxLabelManager {
public:
    ofxLabelManager();
    ~ofxLabelManager();
    
    void    setCamera(ofCamera* _cam);
    void    addLabel(const glm::vec3& _world_pos, const glm::vec3& _screen_center, const std::string& _text);
    
    bool    spaceAt(float _y, bool _left);
    
    void    clear();
    void    update();
    void    draw();
    
private:
    vector<ofxLabel>    m_labels;
    
    ofCamera*           m_cam;
};
