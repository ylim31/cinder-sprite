#pragma once

  // cinder
#include "cinder/gl/Texture.h"
#include "cinder/qtime/QuickTimeGl.h"


enum class provider_type {
  None,
  Image,
  Graphics,
  Video
};

/////////////////////////////////////////////////
//
//  texture_provider
//  Base class for texture providers
//
/////////////////////////////////////////////////
class texture_provider {
public:
  ~texture_provider() {
    source = "";
    ready = false;
    texture.reset();
  }
  
  //////////////////////////////////////////////////////
  // getters
  //////////////////////////////////////////////////////
  
  ci::gl::TextureRef get_texture();
  
  bool has_new_texture();
  
  std::string get_source();
  
  //////////////////////////////////////////////////////
  // properties
  //////////////////////////////////////////////////////
  ci::signals::Signal<void()> texture_update;
  
  //////////////////////////////////////////////////////
  // virtual methods
  //////////////////////////////////////////////////////
  virtual ci::vec2 get_size() = 0;
  
  virtual bool is_ready() = 0;
  
  virtual void set_source(std::string path) = 0;
  
  virtual void update() = 0;
  
  virtual provider_type get_type() { return provider_type::None; }
  
protected:
  //////////////////////////////////////////////////////
  // properties
  //////////////////////////////////////////////////////
  ci::gl::TextureRef texture;
  bool ready;
  std::string source;
  bool texture_is_new;
  bool media_is_looping;
  
  //////////////////////////////////////////////////////
  // methods
  //////////////////////////////////////////////////////
  void set_texture(const ci::gl::TextureRef & new_texture);
};

//////////////////////////////////////////////////////
// typedefs
//////////////////////////////////////////////////////
typedef std::shared_ptr<texture_provider> texture_provider_ref;

/////////////////////////////////////////////////
//
//  image_provider
//
/////////////////////////////////////////////////
class image_provider : virtual public texture_provider {
public:
  //////////////////////////////////////////////////////
  // static
  //////////////////////////////////////////////////////
  typedef std::shared_ptr<image_provider> image_provider_ref;
  
  static image_provider_ref create(std::string filename);
  
  static image_provider_ref create(ci::gl::TextureRef tex);
  
  static image_provider_ref create();
  
  //////////////////////////////////////////////////////
  // ctr(s)
  //////////////////////////////////////////////////////
  image_provider(std::string filename);
  image_provider(ci::gl::TextureRef tex);
  image_provider();
  
  //////////////////////////////////////////////////////
  // getters
  //////////////////////////////////////////////////////
  ci::vec2 get_size() override;
  
  provider_type get_type() override { return provider_type::Image; }
  
  bool is_ready() override;
  
  //////////////////////////////////////////////////////
  // setters
  //////////////////////////////////////////////////////
  void set_source(std::string path) override;
  
  //////////////////////////////////////////////////////
  // methods
  //////////////////////////////////////////////////////
  void update() override {}
};
//////////////////////////////////////////////////////
// typedefs
//////////////////////////////////////////////////////
typedef image_provider::image_provider_ref image_provider_ref;

/////////////////////////////////////////////////
//
//  graphics_provider
//
/////////////////////////////////////////////////
class graphics_provider : public texture_provider {
public:
  typedef std::shared_ptr<graphics_provider> graphics_provider_ref;
  graphics_provider_ref create(ci::vec2 size, bool transparent=true) {
    return std::make_shared<graphics_provider>(size, transparent);
  }

  //////////////////////////////////////////////////////
  // ctr(s)
  //////////////////////////////////////////////////////
  graphics_provider(ci::vec2 size, bool transparent=true);
  
  ~graphics_provider() {
    fbo.reset();
  }
  
  //////////////////////////////////////////////////////
  // getters
  //////////////////////////////////////////////////////
  ci::vec2 get_size() override;
  
  provider_type get_type() override { return provider_type::Graphics; }
  
  //////////////////////////////////////////////////////
  // methods
  //////////////////////////////////////////////////////
  bool is_ready() override;
  
  void set_source(std::string path) override {};
  
  void update() override;
  
  virtual void draw() {};
  
  void set_background(ci::ColorA c);
  
protected:
  ci::gl::FboRef fbo;
  ci::ColorA background;
}; typedef graphics_provider::graphics_provider_ref graphics_provider_ref;

// TODO: Implement platform specific provider for Quicktime & WMFVideoPlayer

/////////////////////////////////////////////////
//
//  VideoProvider
//
/////////////////////////////////////////////////
class video_provider : public texture_provider {
public:
  typedef std::shared_ptr<video_provider> video_provider_ref;
  static video_provider_ref create();
  static video_provider_ref create(ci::fs::path);

  video_provider() {};
  
  video_provider(ci::fs::path);

  virtual ci::vec2 get_size() override;
  
  provider_type get_type() override { return provider_type::Video; }
  
  virtual bool is_ready() override;
  
  virtual void set_source(std::string path) override;
  
  virtual void update() override;
  
  ci::qtime::MovieGlRef get_movie() { return movie; }
  
protected:
  ci::qtime::MovieGlRef movie;
}; typedef video_provider::video_provider_ref video_provider_ref;

