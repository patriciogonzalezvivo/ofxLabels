//
//  ofxLabels.hpp
//
//  Created by Patricio González Vivo on 6/27/18.
//

#pragma once

#include "ofMain.h"

#define MARGIN 25.0
#define HIGHLIGHT_WIDTH_MARGIN 10.0

#define BITMAP_FONT_CHAR_WIDTH 8.0
#define BITMAP_FONT_CHAR_HEIGHT 10.0

#define OCCLUTION_LENGTH 10.0

//#define LABEL_AT_LINE

//#define SDFFONT
//#define FONTSTASH
//#define HERSHEYFONT

#if defined(SDFFONT)
#include "ofxSDFFont.h"
#elif defined(FONTSTASH)
#include "ofxFontStash.h"
#elif defined(HERSHEYFONT)
#include "ofxHersheyFont.h"
#endif

struct ofxLabel {
    string      text;
    glm::vec3   world_position;
    glm::vec3   screen_center;
    glm::vec3   screen_position;
    glm::vec3   screen_proj1;
    glm::vec3   screen_proj2;
    float       width;
    float       height;
    bool        bLeft;
    bool        bTop;
    bool        bVisible;
};

class ofxLabels {
public:
    ofxLabels();
    ~ofxLabels();
    
    void    loadFont(string _name, float _size);
    
    void    setCamera(ofCamera* _cam);
    
    void    setStraightLines(bool _straight) { m_straight = _straight; }
    void    setBackgroundColor(ofFloatColor _color);
    void    setFrontgroundColor(ofFloatColor _color);
    
    void    addLabel(const glm::vec3& _world_pos, const glm::vec3& _screen_center, const std::string& _text);
    
    bool    spaceAt(float _y, bool _left);
    
    void    clear();
    void    update();
    void    draw();
    
private:
    vector<ofxLabel>    m_labels;
    
#if defined(SDFFONT)
    ofxSDFFont          m_font;
#elif defined(FONTSTASH)
    ofxFontStash        m_font;
    float               m_font_size;
#elif defined(HERSHEYFONT)
    ofxHersheyFont      m_font;
    float               m_font_scale;
#endif
    
    ofFloatColor        m_fnt_color;
    ofFloatColor        m_bkg_color;
    
    ofCamera*           m_cam;
    
    bool                m_straight;
    bool                m_bkg;
};
