#pragma once

// cinder
#include "cinder/Timeline.h"
#include "cinder/Vector.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"

// sfmoma
#include "provider.h"
/////////////////////////////////////////////////
//
//  sprite
//  A graphical element for
//  display of a gl::Texture
//
/////////////////////////////////////////////////
class sprite {
public:
  //////////////////////////////////////////////////////
  // enums
  //////////////////////////////////////////////////////
  enum origin_point {
    TopLeft = 0,
    Center = 1
  };
  
  enum provider_type {
    Image,
    Graphics
  };
  
  enum mask_type {
    None,
    ToCenter,
    FromCenter,
    LeftToRight,
    RightToLeft
  };
  
  //////////////////////////////////////////////////////
  // static
  //////////////////////////////////////////////////////
  typedef std::shared_ptr<sprite> sprite_ref;

  static sprite_ref create(provider_type type = provider_type::Image);

  static sprite_ref create(const texture_provider_ref provider_ref);

  static ci::signals::Signal<void()> complete;
  
  static ci::TimelineRef timeline;
  
  static void init();

  //////////////////////////////////////////////////////
  // ctr(s) / dctr(s)
  //////////////////////////////////////////////////////
  sprite(const texture_provider_ref texture_provider);

  sprite(provider_type type = provider_type::Image);

  //////////////////////////////////////////////////////
  // properties
  //////////////////////////////////////////////////////
  texture_provider_ref provider;

  //////////////////////////////////////////////////////
  // getters
  //////////////////////////////////////////////////////
  texture_provider_ref get_provider();
  
  ci::Rectf get_bounds();

  //////////////////////////////////////////////////////
  // setters
  //////////////////////////////////////////////////////
  void set_alpha(float new_alpha);

  void set_coordinates(ci::vec2 new_coords);

  void set_origin(origin_point new_origin);

  void set_provider(texture_provider_ref provider_ref);

  void set_scale(float new_scale);

  void set_source(std::string);

  void set_tint(ci::Color new_tint);

  void set_zoom(float new_zoom);

  void set_zoom_center(ci::vec2 new_focal_point);

  //////////////////////////////////////////////////////
  // methods
  //////////////////////////////////////////////////////
  bool contains_point(ci::vec2 p);
  
  // draw the sprite
  void draw();

  // schedule an alpha animation
  ci::TweenRef<float> alpha_to(
    float target,
    float duration = 0,
    float delay = 0,
    ci::EaseFn fn = ci::easeInOutQuad);

  // schedule a mask animation to hide the sprite
  ci::TweenRef<ci::Rectf> mask_hide(
    mask_type type,
    float duration =0 ,
    float delay = 0,
    ci::EaseFn fn = ci::easeInOutQuad);

  // schedule a mask animation to reveal the sprite
  ci::TweenRef<ci::Rectf> mask_reveal(
    mask_type type,
    float duration = 0,
    float delay = 0,
    ci::EaseFn fn = ci::easeInOutQuad);

  // schedule an animation to move the sprite
  // relative to coords, applied to offset
  ci::TweenRef<ci::vec2> move_to(
    ci::vec2 target,
    float duration = 0,
    float delay = 0,
    ci::EaseFn fn = ci::easeInOutQuad);

  // schedule an animation to scale the sprite
  ci::TweenRef<float> scale_to(
    float target,
    float duration = 0,
    float delay = 0,
    ci::EaseFn fn = ci::easeInOutQuad);

  // schedule an animation to start media
  void start_media(ci::TimelineRef animator, bool loop, bool cue_complete);

  // schedule an animation to tint the sprite
  ci::TweenRef<ci::Color> tint_to(
    ci::Color target,
    float duration = 0,
    float delay = 0,
    ci::EaseFn fn = ci::easeInOutQuad);

  // update the sprite (called every frame)
  void update();

  // schedule an animation to zoom the sprite
  ci::TweenRef<float> zoom_to(
    float target,
    float duration = 0,
    float delay = 0,
    ci::EaseFn fn = ci::easeInOutQuad);

protected:
  //////////////////////////////////////////////////////
  // properties
  //////////////////////////////////////////////////////
  // texture
  ci::Rectf bounds;           // normalized bounds
  ci::gl::FboRef fbo;         // an fbo used in the zoom compositing
  origin_point origin;        // the origin by which to scale and translate this sprite
  ci::gl::TextureRef input;   // the original texture
  ci::gl::TextureRef output;  // zoomed and cropped texture
  ci::vec2 texture_size;      // width and height of the texture
  ci::Area zoom_area;         // an area used to zoom into the image
  ci::vec2 zoom_center;       // the point to zoom into

  // animatables
  ci::Anim<float> alpha;          // alpha channel
  ci::Anim<ci::vec2> coordinates; // offset from absolute coords
  ci::Anim<ci::Rectf> mask;       // rectangular mask
  ci::Anim<float> scale;          // offset from absolute scale
  ci::Anim<ci::Color> tint;       // the tint to be applied to this sprite
  ci::Anim<float> zoom;           // the level of zooming 0 = none, 1.0 completely zoomed in
  
  // handlers
  ci::signals::Connection texture_update_handler;

  //////////////////////////////////////////////////////
  // methods
  //////////////////////////////////////////////////////
  void update_zoom();  // update the zoom area

  void update_fbo();   // update the fbo

  ci::TweenRef<ci::Rectf> apply_mask_animation(
    ci::Rectf mask_start,
    ci::Rectf mask_target,
    float duration, float delay,
    ci::EaseFn fn = ci::easeInOutQuad);   // generic mask animator, used by mask_reveal and mask_hide
};

//////////////////////////////////////////////////////
// typedefs
//////////////////////////////////////////////////////
typedef sprite::sprite_ref sprite_ref;

