//
//  ofxLabels.cpp
//
//  Created by Patricio González Vivo on 6/27/18.
//

#include "ofxLabels.h"

ofxLabels::ofxLabels() {
    
}

ofxLabels::~ofxLabels() {
    
}

void ofxLabels::setCamera(ofCamera* _cam) {
    m_cam = _cam;
}

void ofxLabels::clear(){
    m_labels.clear();
}

void ofxLabels::addLabel(const glm::vec3& _world_pos, const glm::vec3& _screen_center, const std::string& _text) {
    ofxLabel label;
    label.world_position = _world_pos;
    label.screen_center = _screen_center;
    label.text = _text;
    label.width = _text.size() * FONT_CHAR_WIDTH;
    m_labels.push_back(label);
}

bool heightCheck (const ofxLabel& _a, const ofxLabel& _b) {
    return _a.screen_position.y < _b.screen_position.y;
}

bool depthCheck (const ofxLabel& _a, const ofxLabel& _b) {
    return _a.screen_position.z < _b.screen_position.z;
}

void ofxLabels::update() {
    
    // Update the 2D screen position
    for (unsigned int i = 0; i < m_labels.size(); i++) {
        m_labels[i].screen_position = m_cam->worldToScreen(m_labels[i].world_position);
        
        // Is in view? (depth and in viewport)
        if ( m_labels[i].screen_position.z >= 1.0  ||
             m_labels[i].screen_position.x < MARGIN || m_labels[i].screen_position.x > ofGetWidth() - MARGIN ||
             m_labels[i].screen_position.y < MARGIN || m_labels[i].screen_position.y > ofGetHeight() - MARGIN ) {
            m_labels[i].bVisible = false;
        }
        else {
            m_labels[i].bVisible = true;
        }
    }
    
    // Account for depth collisions
    std::sort (m_labels.begin(), m_labels.end(), depthCheck);
    for (unsigned int i = 0; i < m_labels.size(); i++) {
        // Skip non visibles
        if (!m_labels[i].bVisible) {
            continue;
        }
        
        for (int j = i - 1; j >= 0; j--) {
            if (m_labels[j].bVisible) {
                
                // You don't need the z value any more
                m_labels[i].screen_position.z = 0;
                
                // Do they collide on screen space?
                float screen_distance = length(m_labels[i].screen_position - m_labels[j].screen_position);
                if ( screen_distance < OCCLUTION_LENGTH) {
                    m_labels[i].bVisible = false;
                    break;
                }
            }
        }
    }
    
    // Account for label collisions place
    std::sort (m_labels.begin(), m_labels.end(), heightCheck);
    for (unsigned int i = 0; i < m_labels.size(); i++) {

        // Skip non visibles
        if (!m_labels[i].bVisible) {
            continue;
        }
        
        // Right or Left
        m_labels[i].bLeft = m_labels[i].screen_position.x < m_labels[i].screen_center.x;
        
        // Is there space at that height on the screen
#ifndef CHECK_STRAIGHT_FIRST
        bool isFreeSpace = false;
#else
        bool isFreeSpace = true;
        for (int j = i - 1; j >= 0; j--) {
            if (m_labels[j].bVisible && m_labels[i].bLeft == m_labels[j].bLeft) {
                float screen_distance = m_labels[i].screen_position.y - m_labels[j].screen_proj1.y;
                if (abs(screen_distance) < FONT_CHAR_HEIGHT * 3.0) {
                    isFreeSpace = false;
                    break;
                }
            }
        }
#endif
        
        if (isFreeSpace) {
            m_labels[i].screen_proj1 = m_labels[i].screen_position;
            m_labels[i].screen_proj1.z = 0.0;
        }
        else {
            // if there is no space offset diagonally
            glm::vec3 fromFocus = m_labels[i].screen_position - m_labels[i].screen_center;
            glm::vec3 fromFocusDir = glm::normalize(fromFocus);
            m_labels[i].screen_proj1 = m_labels[i].screen_position + fromFocus * 0.5;
            
            m_labels[i].screen_position += fromFocusDir * 10.0;
        }
        
        // it's the first marker inside margin area
        if (m_labels[i].screen_proj1.x < MARGIN || m_labels[i].screen_proj1.x > ofGetWidth() - MARGIN ||
            m_labels[i].screen_proj1.y < MARGIN || m_labels[i].screen_proj1.y > ofGetHeight() - MARGIN ) {
            m_labels[i].bVisible = false;
            continue;
        }
        
        if (m_labels[i].screen_proj1.x < MARGIN + m_labels[i].width) {
            m_labels[i].screen_proj1.x = MARGIN + m_labels[i].width;
        }
        
        if (m_labels[i].screen_proj1.x > ofGetWidth() - MARGIN - m_labels[i].width) {
            m_labels[i].screen_proj1.x = ofGetWidth() - MARGIN - m_labels[i].width;
        }
        
        // Top or Bottom
        m_labels[i].bTop = m_labels[i].screen_proj1.y < m_labels[i].screen_center.y;
        
        if (m_labels[i].bLeft) {
            m_labels[i].screen_proj2.x = MARGIN;
        }
        else {
            m_labels[i].screen_proj2.x = ofGetWidth() - MARGIN;
        }
        
        // Marks on the screen
        if (m_labels[i].screen_proj2.x < MARGIN || m_labels[i].screen_proj2.x > ofGetWidth() - MARGIN ) {
            m_labels[i].bVisible = false;
            continue;
        }
        
        m_labels[i].screen_proj2.y = m_labels[i].screen_proj1.y;
        
        for (int j = i - 1; j >= 0; j--) {
            if (m_labels[i].bLeft == m_labels[j].bLeft) {
                float screen_distance = m_labels[i].screen_proj1.y - m_labels[j].screen_proj1.y;
                if (m_labels[j].bVisible && abs(screen_distance) < FONT_CHAR_HEIGHT * 3.0) {
                    m_labels[i].bVisible = false;
                    break;
                }
            }
        }
    }
}

void ofxLabels::draw() {

    for (unsigned int i = 0; i < m_labels.size(); i++) {
        if ( !m_labels[i].bVisible ) {
            continue;
        }
        
        ofSetColor(170);
        ofDrawLine(m_labels[i].screen_position, m_labels[i].screen_proj1);
        ofDrawLine(m_labels[i].screen_proj1, m_labels[i].screen_proj2);
        ofSetColor(255);
        ofSetDrawBitmapMode(OF_BITMAPMODE_MODEL_BILLBOARD );
        
        glm::vec3 label_pos;
        if (m_labels[i].bLeft) {
            label_pos.x = MARGIN;
        } else {
            label_pos.x = ofGetWidth() - MARGIN - m_labels[i].width;
        }
        
        if (m_labels[i].bTop) {
            label_pos.y = m_labels[i].screen_proj1.y - FONT_CHAR_HEIGHT * 0.75;
        }
        else {
            label_pos.y = m_labels[i].screen_proj1.y + FONT_CHAR_HEIGHT * 1.5;
        }
        
        ofDrawBitmapStringHighlight(m_labels[i].text, label_pos);
    }
}
