//
//  ofxLabels.cpp
//
//  Created by Patricio González Vivo on 6/27/18.
//

#include "ofxLabels.h"

ofxLabels::ofxLabels() {
    m_bkg = false;
}

ofxLabels::~ofxLabels() {
    
}

void ofxLabels::loadFont(string _name, float _size) {
#if defined(SDFFONT)
    m_font.load(_name, _size);
#elif defined(FONTSTASH)
    m_font.setup(_name);
    m_font_size = _size;
#elif defined(HERSHEYFONT)
    m_font.setColor(ofColor(255));
    m_font_scale = _size;
#endif
    
}

void ofxLabels::setBackgroundColor(ofFloatColor _color) {
    m_bkg_color = _color;
    m_bkg = true;
}

void ofxLabels::setFrontgroundColor(ofFloatColor _color) {
    m_fnt_color = _color;
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
    
#if defined(SDFFONT)
    ofRectangle bbox = m_font.getStringRect(_text, ofVec2f(0.,0.));
    label.width = bbox.width;
    label.height = bbox.height;
#elif defined(FONTSTASH)
    ofRectangle bbox = m_font.getBBox(_text, m_font_size, 0, 0);
    label.width = bbox.width;
    label.height = bbox.height;
#elif defined(HERSHEYFONT)
    label.width = m_font.getWidth(_text, m_font_scale);
    label.height = m_font.getHeight( m_font_scale);
#else
    label.width = _text.size() * BITMAP_FONT_CHAR_WIDTH;
    label.height = BITMAP_FONT_CHAR_HEIGHT;
#endif
    
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
#ifdef CHECK_STRAIGHT_FIRST
        bool isFreeSpace = true;
        for (int j = i - 1; j >= 0; j--) {
            if (m_labels[j].bVisible && m_labels[i].bLeft == m_labels[j].bLeft) {
                float screen_distance = m_labels[i].screen_position.y - m_labels[j].screen_proj1.y;
                if (abs(screen_distance) < m_labels[i].height * 2.0) {
                    isFreeSpace = false;
                    break;
                }
            }
        }
#else
        bool isFreeSpace = false;
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
        
        if (m_labels[i].screen_proj1.x < MARGIN + m_labels[i].width + HIGHLIGHT_WIDTH_MARGIN) {
            m_labels[i].screen_proj1.x = MARGIN + m_labels[i].width + HIGHLIGHT_WIDTH_MARGIN;
        }
        
        if (m_labels[i].screen_proj1.x > ofGetWidth() - MARGIN - m_labels[i].width - HIGHLIGHT_WIDTH_MARGIN) {
            m_labels[i].screen_proj1.x = ofGetWidth() - MARGIN - m_labels[i].width - HIGHLIGHT_WIDTH_MARGIN;
        }
        
        // Top or Bottom
        m_labels[i].bTop = m_labels[i].screen_proj1.y < m_labels[i].screen_center.y;
        
        if (m_labels[i].bLeft) {
#ifdef LABEL_AT_LINE
            m_labels[i].screen_proj2.x = MARGIN + m_labels[i].width + 10.0;
#else
            m_labels[i].screen_proj2.x = MARGIN;
#endif
        }
        else {
#ifdef LABEL_AT_LINE
            m_labels[i].screen_proj2.x = ofGetWidth() - MARGIN - m_labels[i].width - 10.0;
#else
            m_labels[i].screen_proj2.x = ofGetWidth() - MARGIN;
#endif
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
                if (m_labels[j].bVisible && abs(screen_distance) < m_labels[j].height * 3.0) {
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
        
        
        
        glm::vec3 label_pos;
        if (m_labels[i].bLeft) {
            label_pos.x = MARGIN;
        } else {
            label_pos.x = ofGetWidth() - MARGIN - m_labels[i].width;
        }
        
#ifdef LABEL_AT_LINE
        label_pos.y = m_labels[i].screen_proj1.y + m_labels[i].height * 0.5;
#else
        if (m_labels[i].bTop) {
            label_pos.y = m_labels[i].screen_proj1.y - m_labels[i].height * 0.75;
        }
        else {
            label_pos.y = m_labels[i].screen_proj1.y + m_labels[i].height * 1.5;
        }
#endif
        
        ofPushStyle();
        ofFill();

        if (m_bkg) {
            ofSetColor(m_bkg_color);
            ofDrawRectangle(label_pos.x - HIGHLIGHT_WIDTH_MARGIN, label_pos.y - m_labels[i].height * 1.5, m_labels[i].width + HIGHLIGHT_WIDTH_MARGIN * 2., m_labels[i].height * 2.);
        }
        
        ofSetColor(m_fnt_color);
        ofDrawLine(m_labels[i].screen_position, m_labels[i].screen_proj1);
        ofDrawLine(m_labels[i].screen_proj1, m_labels[i].screen_proj2);
        
#if defined(SDFFONT)
        m_font.draw(m_labels[i].text, label_pos.x, label_pos.y);
#elif defined(FONTSTASH)
        m_font.draw(m_labels[i].text, m_font_size, label_pos.x, label_pos.y);
#elif defined(HERSHEYFONT)
        m_font.draw(m_labels[i].text, label_pos.x, label_pos.y, m_font_scale);
#else
        ofSetDrawBitmapMode(OF_BITMAPMODE_MODEL_BILLBOARD );
        ofDrawBitmapString(m_labels[i].text, label_pos);
#endif
        ofPopStyle();
    }
}
