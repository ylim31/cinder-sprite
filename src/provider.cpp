  // cinder
#include "cinder/app/App.h"
#include "cinder/Log.h"

  // sfmoma
#include "provider.h"

using namespace ci;
using namespace ci::app;
using namespace ci::qtime;

/////////////////////////////////////////////////
//
//  texture_provider
//  Base class for texture providers
//
/////////////////////////////////////////////////
void texture_provider::set_texture(const gl::TextureRef & newTexture) {
  texture_is_new = true;
  texture = newTexture;
  texture_update.emit();
}

signals::Signal<void()> & texture_provider::get_media_complete_signal() {
  return media_complete_signal;
}

std::string texture_provider::get_source() {
  return source;
}

gl::TextureRef texture_provider::get_texture() {
  texture_is_new = false;
  return texture;
}

bool texture_provider::has_new_texture() {
  return texture_is_new;
}

/////////////////////////////////////////////////
//
//  image_provider
//
/////////////////////////////////////////////////
image_provider_ref image_provider::create(std::string filename){
  return std::make_shared<image_provider>(filename);
}

image_provider_ref image_provider::create(gl::TextureRef tex){
  return std::make_shared<image_provider>(tex);
}

image_provider_ref image_provider::create(){
  return std::make_shared<image_provider>();
}

image_provider::image_provider(std::string filename) {
  set_source(filename);
}

image_provider::image_provider(gl::TextureRef tex) {
  set_texture(tex);
}

image_provider::image_provider() {
  texture_is_new = false;
}

void image_provider::set_source(std::string path) {
  source = path;
  set_texture(gl::Texture::create(ci::loadImage(ci::app::loadAsset(path))));
}

vec2 image_provider::get_size() { 
  return texture->getSize();
}

bool image_provider::is_ready() {
  return texture != nullptr;
}

/////////////////////////////////////////////////
//
//  graphics_provider
//
/////////////////////////////////////////////////
graphics_provider::graphics_provider(ci::vec2 size, bool transparent) {
  gl::Fbo::Format format;
  format.setSamples(4);
  format.setColorTextureFormat(
    gl::Fbo::Format::getDefaultColorTextureFormat(transparent));
  fbo = ci::gl::Fbo::create(size.x, size.y, format);
  background = ColorA(0, 0, 0, 0);
}

vec2 graphics_provider::get_size() {
  return fbo->getSize();
}

bool graphics_provider::is_ready() {
  return true;
};

void graphics_provider::set_background(ColorA c) {
  background = c;
  update();
}

void graphics_provider::update() {
  gl::ScopedFramebuffer scoped_fbo(fbo);
  gl::ScopedMatrices scoped_matrices;
  gl::ScopedViewport scoped_viewport(ivec2(0), fbo->getSize());
  gl::setMatricesWindow(fbo->getSize());
  gl::clear(background);
  draw();
  set_texture(fbo->getColorTexture());
}

/////////////////////////////////////////////////
//
//  VideoProvider
//
/////////////////////////////////////////////////

video_provider_ref video_provider::create() {
  return std::make_shared<video_provider>();
}

video_provider_ref video_provider::create(fs::path p) {
  return std::make_shared<video_provider>(p);
}

video_provider::video_provider(fs::path p) {
  movie = MovieGl::create(p);
}

vec2 video_provider::get_size() {
  if(movie) return movie->getSize();
  return vec2(512, 512);
};

bool video_provider::is_ready() {
  if(!movie) return false;
  return movie->isPlayable();
}

void video_provider::set_source(std::string path) {
  if(movie) {
    movie->stop();
  }
  
  movie = MovieGl::create(path);
  movie->play();
};

void video_provider::update() {
  if(movie) {
    if(movie->isPlaying()) {
      if(movie->checkNewFrame()) {
        set_texture(movie->getTexture());
      }
    }
  }
}
