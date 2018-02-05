#pragma once

#include "cinder/gl/Texture.h"

class texture_resizer {
public:
  class options {
  public:
    enum fit {
      Scale,
      Crop
    };
    
    options(fit fit, ci::vec2 scale) : fit_type(fit), bounds_scale(scale) {};
    
    fit get_fit() { return fit_type; }
    ci::vec2 get_bounds_scale() { return bounds_scale; }
    
  private:
    ci::vec2 bounds_scale;
    fit fit_type;
  };
  
  class result {
  public:
    result(ci::gl::TextureRef result, ci::Rectf bounding_box) : texture(result), bounds(bounding_box) {};
    
    ci::gl::TextureRef get_texture() { return texture; }
    
    ci::Rectf get_bounds() { return bounds; }
    
  private:
    ci::gl::TextureRef texture;
    ci::Rectf bounds;
  };
  
  static result process(
                        ci::gl::TextureRef input, ci::vec2 screen_size,
                        float top_left[2], float bottom_right[2], texture_resizer::options options);
  
  static ci::Rectf createBoundingRect(float top_left[2], float bottom_right[2], ci::vec2 screen_size);
};
