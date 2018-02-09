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

sprite_ref sprite::create(texture_provider_ref provider){
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
  scale = 1.0f;
  scale_offset() = 1.0f;
  tint() = Color::white();
  provider = texture_provider;
  zoom() = 0.0f;
}

sprite::sprite(provider_type type) {
  alpha() = 1.0f;
  origin = origin_point::TopLeft;
  scale = 1.0f;
  scale_offset() = 1.0f;
  tint() = Color::white();
  zoom() = 0.0f;
  
  switch(type) {
    case provider_type::Image:
      provider = image_provider::create();
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
}

void sprite::set_scale(float new_scale) {
  scale_offset() = std::max(0.0f, new_scale);
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
texture_provider_ref sprite::get_provider() {
  return provider;
}

//////////////////////////////////////////////////////
// methods
//////////////////////////////////////////////////////
ci::TweenRef<float> sprite::alpha_to(TimelineRef animator, float target, float duration, float delay, EaseFn ease_fn) {
  if (duration <= 0) {
    alpha = 0;
    return nullptr;
  } else {
    return animator->appendTo(&alpha, target, duration).delay(delay).easeFn(ease_fn);
  }
}

void sprite::apply_mask_animation(TimelineRef animator, Rectf startMask, Rectf targetMask, float duration, float delay, EaseFn easeFn) {
  animator->appendTo(&mask_rect, startMask, targetMask, duration).delay(delay).easeFn(easeFn);
}

void sprite::draw() {
  if (alpha() > 0.0 && crop) {
    gl::ScopedColor sc;
    gl::ScopedBlendAlpha sa;
    gl::ScopedModelMatrix m1;
    gl::translate(coordinates + offset());
    gl::scale(scale * scale_offset(), scale * scale_offset());
    if(origin == origin_point::Center) {
      gl::translate(-texture_size * 0.5f);
    }
    gl::color(ColorA(tint, alpha));
    gl::draw(crop, Area(mask_rect), mask_rect);
  }
}

void sprite::mask_hide(TimelineRef animator, std::string animation, float duration, float delay, EaseFn ease_fn) {
  if (animation == "none") {
    animator->appendTo(&mask_rect, Rectf(0, 0, 0, 0), 0.0f).delay(delay);
  }

  if (animation == "left-to-right") {
    apply_mask_animation(animator, Rectf(bounds), Rectf(bounds.x2, bounds.y1, bounds.x2, bounds.y2), duration, delay, ease_fn);
  }

  if (animation == "right-to-left") {
    apply_mask_animation(animator, Rectf(bounds), Rectf(bounds.x1, bounds.y1, bounds.x1, bounds.y2), duration, delay, ease_fn);
  }

  if (animation == "to-center") {
    Rectf end(bounds);
    end.scaleCentered(0.0);
    apply_mask_animation(animator, Rectf(bounds), end, duration, delay, ease_fn);
  }
}

void sprite::mask_reveal(TimelineRef animator, std::string animation, float duration, float delay, EaseFn ease_fn) {
  if (animation == "none") {
    animator->appendTo(&mask_rect, Rectf(vec2(0), texture_size), 0.0f).delay(delay);
  }

  if (animation == "left-to-right") {
    apply_mask_animation(animator, Rectf(bounds.x1, bounds.y1, bounds.x1, bounds.y2), Rectf(bounds), duration, delay, ease_fn);
  }

  if (animation == "right-to-left") {
    apply_mask_animation(animator, Rectf(bounds.x2, bounds.y1, bounds.x2, bounds.y2), Rectf(bounds), duration, delay, ease_fn);
  }

  if (animation == "from-center") {
    Rectf start(bounds);
    start.scaleCentered(0.0);
    apply_mask_animation(animator, Rectf(start), Rectf(bounds), duration, delay, ease_fn);
  }
}

/**
 * Invokes animation on coordicates
 */
ci::TweenRef<vec2> sprite::move_to(TimelineRef animator, vec2 target, float duration, float delay, EaseFn ease_fn) {
  if (duration <= 0) {
    offset = target;
    return nullptr;
  } else {
    return animator->appendTo(&offset, target, duration).delay(delay).easeFn(ease_fn);
  }
}

/**
 * Invokes animation on scale
 */
ci::TweenRef<float> sprite::scale_to(TimelineRef animator, float target, float duration, float delay, ci::EaseFn ease_fn) {
  if (duration <= 0) {
    scale_offset = target;
    return nullptr;
  } else {
    return animator->appendTo(&scale_offset, target, duration).delay(delay).easeFn(ease_fn);
  }
}

/**
 * Starts media playback from provider
 */
void sprite::start_media(TimelineRef animator, bool loop, bool cue_complete) {
  provider->start_media(loop);

  if (cue_complete) {
    provider->get_media_complete_signal().connect([&] {
      sprite::complete.emit();
    });
  }
}

ci::TweenRef<ci::Color> sprite::tint_to(TimelineRef animator, Color target, float duration, float delay, EaseFn ease_fn) {
  if (duration <= 0) {
    tint = target;
    return nullptr;
  } else {
    return animator->appendTo(&tint, target, duration).delay(delay).easeFn(ease_fn);
  }
}

/** 
 * Call once per frame
 */
void sprite::update() {
  if (provider) {
    provider->update();
    if (provider->has_new_texture()) {
      // set the new texture
      texture = provider->get_texture();
      // if the size of the texture changes we need to update all size related vars
      if (texture_size != provider->get_size()) {

        // set the new texture size
        texture_size = provider->get_size();

        // update the mask rects to conform to the new
        bounds.set(0, 0, texture_size.x, texture_size.y);
        mask_rect = Rectf(bounds.x1, bounds.y1, bounds.x2, bounds.y2);

        // need to update the zoom area if the texture size has changed
        zoom_center = texture_size * 0.5f;
        update_zoom();
      }
      update_fbo();
    }
  }
}

void sprite::update_fbo() {
  if (texture) {
    if (!fbo) {
      fbo = gl::Fbo::create(texture_size.x, texture_size.y, true);
    }

    gl::ScopedMatrices scoped_matrices;
    gl::ScopedFramebuffer scoped_fbo(fbo);
    gl::ScopedViewport scoped_viewport(ivec2(0), fbo->getSize());
    gl::setMatricesWindow(fbo->getSize());
    gl::clear(ColorA(0, 0, 0, 0));
    gl::draw(texture, zoom_area, fbo->getBounds());
    crop = fbo->getColorTexture();
  }
}

void sprite::update_zoom() {
  float z = 1.0f - zoom;
  vec2 ul = zoom_center - texture_size * 0.5f * z;
  vec2 lr = zoom_center + texture_size * 0.5f * z;
  zoom_area = Area(ul, lr);
}

ci::TweenRef<float> sprite::zoom_to(TimelineRef animator, float target, float duration, float delay, EaseFn ease_fn) {
  if (duration <= 0) {
    zoom = target;
    update_zoom();
    update_fbo();
    return nullptr;
  } else {
    return animator->appendTo(&zoom, glm::clamp(target, 0.0f, 1.0f), duration).delay(delay).easeFn(ease_fn).updateFn([&]{
      update_zoom();
      update_fbo();
    });
  }
}
