// std
#include <algorithm>

// cinder
#include "cinder/app/App.h"
#include "cinder/Log.h"
#include "cinder/gl/gl.h"

  // sfmoma
#include "sprite.h"
#include "provider.h"

  //#include "webview/JSDelegate.h"
  //#include "webview/MethodDispatcher.h"

using namespace ci;
using namespace ci::app;

  ////////////////////////////////////////////////////
  //  static
  ////////////////////////////////////////////////////
ci::signals::Signal<void()> sprite::complete;

sprite_ref sprite::create(texture_provider_ref provider){
  return std::make_shared<sprite>(provider);
}

sprite_ref sprite::create() {
  return std::make_shared<sprite>();
}

std::map<std::string, EaseFn> sprite::easing = { 
  { "ease-in-quad", EaseInQuad() },
  { "ease-out-quad", EaseOutQuad() },
  { "ease-in-out-quad", EaseInOutQuad() },
  { "ease-out-in-quad", EaseOutInQuad() },
  { "ease-in-cubic", EaseInCubic() },
  { "ease-out-cubic", EaseOutCubic() },
  { "ease-in-out-cubic", EaseInOutCubic() },
  { "ease-out-in-cubic", EaseOutInCubic() },
  { "ease-in-quart", EaseInQuart() },
  { "ease-out-quart", EaseOutQuart() },
  { "ease-in-out-quart", EaseInOutQuart() },
  { "ease-out-in-quart", EaseOutInQuart() },
  { "ease-in-quint", EaseInQuint() },
  { "ease-out-quint", EaseOutQuint() },
  { "ease-in-out-quint", EaseOutQuint() },
  { "ease-out-in-quint", EaseOutInQuint() },
  { "ease-in-sine", EaseInSine() },
  { "ease-out-sine", EaseOutSine() },
  { "ease-in-out-sine", EaseInOutSine() },
  { "ease-out-in-sine", EaseOutInSine() },
  { "ease-in-expo", EaseInExpo() },
  { "ease-out-expo", EaseOutExpo() },
  { "ease-in-out-expo", EaseInOutExpo() },
  { "ease-out-in-expo", EaseOutInExpo() },
  { "ease-in-circ", EaseInCirc() },
  { "ease-out-circ", EaseOutCirc() },
  { "ease-in-out-circ", EaseInOutCirc() },
  { "ease-out-in-circ", EaseOutInCirc() },
  { "ease-in-atan", EaseInAtan() },
  { "ease-out-atan", EaseOutAtan() },
  { "ease-in-out-atan", EaseInOutAtan() },
  { "ease-none", EaseNone() },
  { "ease-in-back", EaseInBack() },
  { "ease-out-back", EaseOutBack() },
  { "ease-in-out-back", EaseInOutBack() },
  { "ease-out-in-back", EaseOutInBack() },
  { "ease-in-bounce", EaseInBounce() },
  { "ease-out-bounce", EaseOutBounce() },
  { "ease-in-out-bounce", EaseInOutBounce() },
  { "ease-out-in-bounce", EaseOutInBounce() }
};


EaseFn sprite::look_up_easing_function(std::string key) {
  if (easing.count(key) != 0) {
    return easing.at(key);
  }
  return easeInOutQuad;
}

  //////////////////////////////////////////////////////
  // ctr(s) / dctr(s)
  //////////////////////////////////////////////////////
sprite::sprite(const texture_provider_ref texture_provider) {
  provider = texture_provider;
  zoom() = 0.0f;
  alpha() = 1.0f;
  scale = 1.0f;
  scale_offset() = 1.0f;
  tint() = Color::white();
}

sprite::sprite() {
  zoom() = 0.0f;
  alpha() = 1.0f;
  scale = 1.0f;
  scale_offset() = 1.0f;
  tint() = Color::white();
}

//////////////////////////////////////////////////////
// setters
//////////////////////////////////////////////////////
void sprite::set_alpha(float new_alpha) {
  alpha = std::max(0.0f, std::min(new_alpha, 1.0f));
}

/**
 * Sets the sprites coordinates.
 * This moves the sprite right away and does not apply animation
 * Further, it sets the absolute coordinates from which
 * subsequent calls to moveTo will be relative to.
 */
void sprite::set_coordinates(vec2 new_coordinates) {
  coordinates = new_coordinates;
}


void sprite::set_provider(texture_provider_ref provider_ref) {
  provider = provider_ref;
}

/**
 * Sets the sprites scale
 * This scales the sprite right away and does not apply animation
 * Further, it sets the absolute scale from which
 * subsequent calls to scaleTo will be relative to.
 */
void sprite::set_scale(float new_scale) {
  scale_offset() = std::max(0.0f, new_scale);
}

void sprite::set_tint(Color new_color) {
  tint() = new_color;
}

void sprite::set_zoom_center(vec2 new_zoom_center) {
  zoom_center = new_zoom_center;
  update_zoom();
}

void sprite::set_zoom(float new_zoom) {
  zoom_to(nullptr, new_zoom, 0);
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
/**
 * Starts media playback from provider
 */
void sprite::alpha_to(TimelineRef animator, float target, float duration, float delay, EaseFn ease_fn, bool signal_complete) {
  if (duration <= 0) {
    alpha = 0;
    if (signal_complete) {
      sprite::complete.emit();
    }
  }
  else {
    animator->appendTo(&alpha, target, duration).delay(delay).easeFn(ease_fn).finishFn([&, signal_complete] {
      if (signal_complete) {
        sprite::complete.emit();
      }
    });
  }
}

/**
 * Convenience method for invoking an animation on the mask
 */
void sprite::apply_mask_animation(
                                TimelineRef animator,
                                Rectf startMask, Rectf targetMask,
                                float duration, float delay, EaseFn easeFn, bool signalComplete) {
  
  animator->appendTo(&mask_rect, startMask, targetMask, duration).delay(delay).easeFn(easeFn).finishFn([&, signalComplete]{
    if (signalComplete) {
      sprite::complete.emit();
    }
  });
}

/**
 * Draws texture to window
 * Call once per frame, per window
 */
void sprite::draw() {
  if (alpha() > 0.0 && crop) {
    gl::ScopedColor sc;
    gl::ScopedBlendAlpha sa;
    gl::ScopedModelMatrix m1;
    gl::translate(coordinates + offset());
    gl::scale(scale * scale_offset(), scale * scale_offset());
    gl::translate(-texture_size * 0.5f);
    gl::color(ColorA(tint, alpha));
    gl::draw(crop, Area(mask_rect), mask_rect);
  }
}

/**
 * Hides this sprite with a mask
 */
void sprite::mask_hide(
                      TimelineRef animator, std::string animation,
                      float duration, float delay, EaseFn ease_fn, bool signalComplete) {
  
  if (animation == "none") {
    animator->appendTo(&mask_rect, Rectf(0, 0, 0, 0), 0.0f).delay(delay).finishFn([&, signalComplete] {
      if (signalComplete) {
        sprite::complete.emit();
      }
    });
  }
  
  if (animation == "left-to-right") {
    apply_mask_animation(
                       animator,
                       Rectf(bounds),
                       Rectf(bounds.x2, bounds.y1, bounds.x2, bounds.y2),
                       duration, delay, ease_fn);
  }
  
  if (animation == "right-to-left") {
    apply_mask_animation(
                       animator,
                       Rectf(bounds),
                       Rectf(bounds.x1, bounds.y1, bounds.x1, bounds.y2),
                       duration, delay, ease_fn);
  }
  
  if (animation == "to-center") {
    Rectf end(bounds);
    end.scaleCentered(0.0);
    apply_mask_animation(
                       animator,
                       Rectf(bounds), end,
                       duration, delay, ease_fn);
  }
}

/**
 * Reveals this sprite with a mask reveal
 */
void sprite::mask_reveal(
                        TimelineRef animator, std::string animation,
                        float duration, float delay, EaseFn ease_fn, bool signal_complete) {
  
  if (animation == "none") {
    animator->appendTo(&mask_rect, Rectf(vec2(0), texture_size), 0.0f).delay(delay).finishFn([&, signal_complete] {
      if (signal_complete) {
        sprite::complete.emit();
      }
    });
  }
  
  if (animation == "left-to-right") {
    apply_mask_animation(
                       animator,
                       Rectf(bounds.x1, bounds.y1, bounds.x1, bounds.y2),
                       Rectf(bounds), duration, delay, ease_fn);
  }
  
  if (animation == "right-to-left") {
    apply_mask_animation(
                       animator,
                       Rectf(bounds.x2, bounds.y1, bounds.x2, bounds.y2),
                       Rectf(bounds), duration, delay, ease_fn);
  }
  
  if (animation == "from-center") {
    Rectf start(bounds);
    start.scaleCentered(0.0);
    apply_mask_animation(
                       animator,
                       Rectf(start),
                       Rectf(bounds), duration, delay, ease_fn);
  }
}

/**
 * Invokes animation on coordicates
 */
void sprite::move_to(TimelineRef animator, vec2 target, float duration, float delay, EaseFn ease_fn, bool signal_complete) {
  if (duration <= 0) {
    offset = target;
    if (signal_complete) {
      sprite::complete.emit();
    }
  }
  else {
    animator->appendTo(&offset, target, duration).delay(delay).easeFn(ease_fn).finishFn([&, signal_complete]{
      if (signal_complete) {
        sprite::complete.emit();
      }
    });
  }
}

/**
 * Invokes animation on scale
 */
ci::TweenRef<float> sprite::scale_to(TimelineRef animator, float target, float duration, float delay, ci::EaseFn ease_in, bool signal_complete) {
  if (duration <= 0) {
    scale_offset = target;
    if (signal_complete) {
      sprite::complete.emit();
    }
  }
  else {
    return animator->appendTo(&scale_offset, target, duration).delay(delay).easeFn(ease_in)
    .finishFn([&, signal_complete]{
      if (signal_complete) {
        sprite::complete.emit();
      }
    });
  }
}

/**
 * Starts media playback from provider
 */
void sprite::start_media(TimelineRef animator, bool loop, bool cue_complete, bool cue_complete_transition) {
  provider->start_media(loop);
  
  if (cue_complete) {
    provider->get_media_complete_signal().connect([&] {
      sprite::complete.emit();
    });
  }
  
  if (cue_complete_transition) {
    provider->get_media_complete_signal().connect([&, animator] {
      alpha_to(animator, 0.0f, 2.0f);
    });
  }
}

void sprite::tint_to(TimelineRef animator, Color target, float duration, float delay, EaseFn ease_fn, bool signal_complete) {
  if (duration <= 0) {
    tint = target;
    if (signal_complete) {
      sprite::complete.emit();
    }
  }
  else {
    animator->appendTo(&tint, target, duration).delay(delay).easeFn(ease_fn).finishFn([&, signal_complete] {
      if (signal_complete) {
        sprite::complete.emit();
      }
    });
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


/**
 * Resets the area to zoom into.
 * This should be called whenver textureSize or zoom parameters are set
 * Creates a crop of the image zoomed into a specific point
 */
void sprite::update_zoom() {
  float z = 1.0f - zoom;
  vec2 ul = zoom_center - texture_size * 0.5f * z;
  vec2 lr = zoom_center + texture_size * 0.5f * z;
  zoom_area = Area(ul, lr);
}

/**
 * Invokes an animation on the zoom
 */
void sprite::zoom_to(TimelineRef animator, float target, float duration, float delay, EaseFn ease_fn, bool signal_complete) {
  if (duration <= 0) {
    zoom = target;
    update_zoom();
    update_fbo();
    if (signal_complete) {
      sprite::complete.emit();
    }
  }
  else {
    animator->appendTo(&zoom, glm::clamp(target, 0.0f, 1.0f), duration).delay(delay).easeFn(ease_fn).updateFn([&]{
      update_zoom();
      update_fbo();
    }).finishFn([&, signal_complete] {
      if (signal_complete) {
        sprite::complete.emit();
      }
    });
  }
}
