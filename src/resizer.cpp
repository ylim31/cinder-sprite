  // cinder
#include "cinder/app/App.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/gl.h"

  // sfmoma
#include "resizer.h"

using namespace ci;
using namespace ci::app;

Rectf texture_resizer::createBoundingRect(float top_left[2], float bottom_right[2], vec2 screen_size) {
  Rectf bounds;
  float w = screen_size.x;
  float h = screen_size.y;
  vec2 tl(top_left[1] * w, top_left[0] * h);
  vec2 br(bottom_right[1] * w + w, bottom_right[0] * h + h);
  bounds.set(tl.x, tl.y, br.x, br.y);
  return bounds;
}

texture_resizer::result texture_resizer::process(
  gl::TextureRef input, vec2 screen_size, float top_left[2], float bottom_right[2], texture_resizer::options options) {
  
  Rectf crop;
  float scale = 1.0f;
  float aspect_ratio = 1.0f;
  Rectf bounds = createBoundingRect(top_left, bottom_right, screen_size);
  bounds.scaleCentered(options.get_bounds_scale());
  crop.set(0, 0, bounds.getWidth(), bounds.getHeight());
  
  switch (options.get_fit()){
    case options::fit::Crop:
      aspect_ratio = crop.getWidth() / crop.getHeight();
      if (aspect_ratio > 1.0f) {
          // landscape
        scale = input->getWidth() / crop.getWidth();
        crop.scale(vec2(scale, scale));
        
        if (input->getHeight() < crop.getHeight()) {
          scale = input->getHeight() / crop.getHeight();
          crop.scale(vec2(scale, scale));
        }
      }
      else {
          // portrait
        scale = input->getHeight() / crop.getHeight();
        crop.scale(vec2(scale, scale));
        
        if (input->getWidth() < crop.getWidth()) {
          scale = input->getWidth() / crop.getWidth();
          crop.scale(vec2(scale, scale));
        }
      }
      
        // center the crop on the image
      crop.offsetCenterTo(ivec2((float)input->getWidth() * 0.5, (float)input->getHeight() * 0.5));
      break;
      
    case options::fit::Scale:
      aspect_ratio = (float)input->getWidth() / (float)input->getHeight();
      if (aspect_ratio > 1.0f) {
          // landscape
        scale = bounds.getWidth() / input->getWidth();
        if (input->getHeight() * scale > bounds.getHeight()) {
          scale = bounds.getHeight() / input->getHeight();
        }
      }
      else {
        scale = bounds.getHeight() / input->getHeight();
        if (input->getWidth() * scale > bounds.getWidth()) {
          scale = bounds.getWidth() / input->getWidth();
        }
      }
      
      vec2 new_size = (vec2)input->getSize() * scale;
      Rectf new_bounds = Rectf(0, 0, new_size.x, new_size.y);
      new_bounds.offsetCenterTo(bounds.getCenter());
      bounds = new_bounds;
      bounds.scaleCentered(options.get_bounds_scale());
      crop.set(0, 0, input->getWidth(), input->getHeight());
      break;
  }
  
  gl::FboRef fbo = gl::Fbo::create(bounds.getWidth(), bounds.getHeight(), true);
  {
    gl::ScopedFramebuffer scoped_fbo(fbo);
    gl::ScopedViewport scoped_viewport(ivec2(0), fbo->getSize());
    gl::setMatricesWindow(fbo->getSize());
    gl::clear(ColorA(0, 0, 0, 0));
    gl::draw(input, Area(crop), fbo->getBounds());
  }
  return  texture_resizer::result(fbo->getColorTexture(), bounds);
}
