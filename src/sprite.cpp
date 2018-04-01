// std
#include <algorithm>

// cinder
#include "cinder/app/App.h"
#include "cinder/Log.h"
#include "cinder/gl/gl.h"

// sfmoma
#include "sprite.h"
#include "provider.h"

using namespace ci;
using namespace ci::app;

////////////////////////////////////////////////////
//  static
////////////////////////////////////////////////////
ci::signals::Signal<void()> sprite::complete;

sprite_ref sprite::create(const texture_provider_ref provider){
  return std::make_shared<sprite>(provider);
}

sprite_ref sprite::create(provider_type type) {
  return std::make_shared<sprite>(type);
}

//////////////////////////////////////////////////////
// ctr(s) / dctr(s)
//////////////////////////////////////////////////////
sprite::sprite(const texture_provider_ref texture_provider) {
  alpha() = 1.0f;
  origin = origin_point::TopLeft;
  scale() = vec2(1.0f);
  tint() = Color::white();
  zoom() = 0.0f;
  
  set_provider(texture_provider);
}

sprite::sprite(provider_type type) {
  alpha() = 1.0f;
  origin = origin_point::TopLeft;
  scale() = vec2(1.0f);
  tint() = Color::white();
  zoom() = 0.0f;
  
  // TODO: Create default create methods for each provider type
  switch(type) {
    case provider_type::Image:
      set_provider(image_provider::create());
      break;
    default:
      break;
  }
}

//////////////////////////////////////////////////////
// setters
//////////////////////////////////////////////////////
void sprite::set_alpha(float new_alpha) {
  alpha = std::max(0.0f, std::min(new_alpha, 1.0f));
}

void sprite::set_coordinates(vec2 new_coordinates) {
  coordinates = new_coordinates;
}

void sprite::set_origin(origin_point new_origin) {
  origin = new_origin;
}

void sprite::set_provider(texture_provider_ref provider_ref) {
  provider = provider_ref;
  
  if(texture_update_handler.isConnected()) {
    texture_update_handler.disable();
    texture_update_handler.disconnect();
  }
  
  // Add a handler for texture updates from provider
  texture_update_handler = provider->texture_update.connect(
    std::bind(&sprite::on_provider_texture_update, this));
  
  if(provider->is_ready()) {
    on_provider_texture_update();
  }
}

void sprite::set_scale(float new_scale) {
  //scale() = std::max(0.0f, new_scale);
  set_scale(vec2(new_scale, new_scale));
}

void sprite::set_scale(vec2 new_scale) {
  scale() = vec2(std::max(0.0f, new_scale.x), std::max(0.0f, new_scale.y));
}

void sprite::set_source(std::string source) {
  if(provider) {
    provider->set_source(source);
  } else {
    CI_LOG_W("Provider is null. Can not set source: " << source);
  }
}

void sprite::set_tint(Color new_color) {
  tint() = new_color;
}

void sprite::set_zoom_center(vec2 new_zoom_center) {
  zoom_center = new_zoom_center;
  update_zoom();
}

void sprite::set_zoom(float new_zoom) {
  zoom = new_zoom;
  update_zoom();
  update_fbo();
}

//////////////////////////////////////////////////////
// getters
//////////////////////////////////////////////////////
ci::Rectf sprite::get_bounds() {
  ci::Rectf b = Rectf(bounds);
  b.offset(coordinates);
  if(origin == origin_point::Center) b.offset(-bounds.getSize() * 0.5f);
  b.scaleCentered(scale());
  return b;
}

texture_provider_ref sprite::get_provider() {
  return provider;
}

//////////////////////////////////////////////////////
// methods
//////////////////////////////////////////////////////
ci::TweenRef<float> sprite::alpha_to(float target, float duration, float delay, EaseFn ease_fn, bool append) {
  if (duration <= 0) {
    alpha = 0;
    return nullptr;
  } else {
    if(append) return ci::app::timeline().appendTo(&alpha, target, duration).delay(delay).easeFn(ease_fn);
    return ci::app::timeline().apply(&alpha, target, duration).delay(delay).easeFn(ease_fn);
  }
}

ci::TweenRef<ci::Rectf> sprite::apply_mask_animation(Rectf startMask, Rectf targetMask, float duration, float delay, EaseFn easeFn) {
  return ci::app::timeline().apply(&mask, startMask, targetMask, duration).delay(delay).easeFn(easeFn);
}

bool sprite::contains_point(const ci::vec2 & p) {
  ci::Rectf b(bounds);
  b.offset(coordinates());
  if(origin == origin_point::Center) b.offset(-bounds.getSize() * 0.5f);
  b.scaleCentered(scale());
  return b.contains(p);
}

void sprite::draw() {
  if(alpha() > 0.0 && output) {
    gl::ScopedMatrices m1;
    gl::translate(coordinates());
    gl::scale(scale());
    if(origin == origin_point::Center) {
      gl::translate(-texture_size * 0.5f);
    }
    gl::ScopedColor sc;
    gl::ScopedBlendAlpha sa;
    gl::color(ColorA(tint, alpha));
    gl::draw(output, Area(mask), mask);
  }
}

ci::TweenRef<ci::Rectf> sprite::mask_hide(mask_type type, float duration, float delay, EaseFn ease_fn) {
  switch(type) {
    case mask_type::ToCenter: {
      Rectf end(bounds);
      end.scaleCentered(0.0);
      return apply_mask_animation(Rectf(bounds), end, duration, delay, ease_fn);
    }
    case mask_type::LeftToRight: {
      return apply_mask_animation(
        Rectf(bounds), Rectf(
          bounds.x2, bounds.y1, bounds.x2, bounds.y2), duration, delay, ease_fn);
    }
    case mask_type::RightToLeft: {
      return apply_mask_animation(
        Rectf(bounds), Rectf(
          bounds.x1, bounds.y1, bounds.x1, bounds.y2), duration, delay, ease_fn);
    }
    default:
      return ci::app::timeline().apply(&mask, Rectf(0, 0, 0, 0), 0.0f).delay(delay);
  }
}

ci::TweenRef<ci::Rectf> sprite::mask_reveal(mask_type type, float duration, float delay, EaseFn ease_fn) {
  switch(type) {
    case mask_type::FromCenter: {
      Rectf start(bounds);
      start.scaleCentered(0.0);
      return apply_mask_animation(Rectf(start), Rectf(bounds), duration, delay, ease_fn);
    }
    case mask_type::LeftToRight: {
      return apply_mask_animation(
        Rectf(bounds.x1, bounds.y1, bounds.x1, bounds.y2),
          Rectf(bounds), duration, delay, ease_fn);
    }
    case mask_type::RightToLeft: {
      return apply_mask_animation(
        Rectf(bounds.x2, bounds.y1, bounds.x2, bounds.y2),
          Rectf(bounds), duration, delay, ease_fn);
    }
    default:
      return ci::app::timeline().appendTo(&mask, Rectf(vec2(0), texture_size), 0.0f).delay(delay);
  }
}

/**
 * Invokes animation on coordicates
 */
ci::TweenRef<vec2> sprite::move_to(vec2 target, float duration, float delay, EaseFn ease_fn, bool append) {
  if (duration <= 0) {
    coordinates = target;
    return nullptr;
  } else {
    if(append) return ci::app::timeline().appendTo(&coordinates, target, duration).delay(delay).easeFn(ease_fn);
    return ci::app::timeline().apply(&coordinates, target, duration).delay(delay).easeFn(ease_fn);
  }
}

/**
 * Invokes animation on scale
 */
ci::TweenRef<vec2> sprite::scale_to(ci::vec2 target, float duration, float delay, ci::EaseFn ease_fn) {
  if (duration <= 0) {
    scale() = target;
    return nullptr;
  } else {
    return ci::app::timeline().apply(&scale, target, duration).delay(delay).easeFn(ease_fn);
  }
}

ci::TweenRef<vec2> sprite::scale_to(float target, float duration, float delay, ci::EaseFn ease_fn) {
  if (duration <= 0) {
    scale() = vec2(target);
    return nullptr;
  } else {
    return ci::app::timeline().apply(&scale, vec2(target), duration).delay(delay).easeFn(ease_fn);
  }
}

ci::TweenRef<ci::Color> sprite::tint_to(Color target, float duration, float delay, EaseFn ease_fn) {
  if (duration <= 0) {
    tint = target;
    return nullptr;
  } else {
    return ci::app::timeline().apply(&tint, target, duration).delay(delay).easeFn(ease_fn);
  }
}

/**
 * Handles a texture change in the provider
 */
void sprite::on_provider_texture_update() {
  if (provider && provider->has_new_texture()) {
    // get the updated texture
    input = provider->get_texture();

    // if the size of the texture changes we need to update all size related vars
    if (texture_size != provider->get_size()) {

      // set the new texture size
      texture_size = provider->get_size();

      // update the bounds mask and zoom
      bounds.set(0, 0, texture_size.x, texture_size.y);
      mask = Rectf(bounds.x1, bounds.y1, bounds.x2, bounds.y2);
      zoom_center = texture_size * 0.5f;
      update_zoom();
      
      // since the size changed, we set the fbo to null, which will
      fbo = gl::Fbo::create(texture_size.x, texture_size.y, true);
    }
    
    // ...and finally
    update_fbo();
  }
}

/**
 * Update the contents of this sprite
 */
void sprite::update_fbo() {
  if (input) {
    if (!fbo) {
      fbo = gl::Fbo::create(texture_size.x, texture_size.y, true);
    }
    
    gl::ScopedMatrices scoped_matrices;
    gl::ScopedFramebuffer scoped_fbo(fbo);
    gl::ScopedViewport scoped_viewport(ivec2(0), fbo->getSize());
    gl::setMatricesWindow(fbo->getSize());
    gl::clear(ColorA(0, 0, 0, 0));
    gl::ScopedBlendPremult pre;
    gl::draw(input, zoom_area, fbo->getBounds());
    output = fbo->getColorTexture();
  }
}

void sprite::update_zoom() {
  float z = 1.0f - zoom;
  vec2 ul = zoom_center - texture_size * 0.5f * z;
  vec2 lr = zoom_center + texture_size * 0.5f * z;
  zoom_area = Area(ul, lr);
}

ci::TweenRef<float> sprite::zoom_to(float target, float duration, float delay, EaseFn ease_fn) {
  if (duration <= 0) {
    zoom = target;
    update_zoom();
    update_fbo();
    return nullptr;
  } else {
    return ci::app::timeline().apply(&zoom, glm::clamp(target, 0.0f, 1.0f), duration).delay(delay).easeFn(ease_fn).updateFn([&]{
      update_zoom();
      update_fbo();
    });
  }
}
