  // cinder
#include "cinder/app/App.h"
#include "cinder/Log.h"

  // sfmoma
#include "provider.h"

using namespace ci;
using namespace ci::app;

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
  format.setSamples(8);
  fbo = ci::gl::Fbo::create(size.x, size.y, format);
  
}

vec2 graphics_provider::get_size() {
  return fbo->getSize();
}

bool graphics_provider::is_ready() {
  return true;
};

void graphics_provider::update() {
  gl::ScopedFramebuffer scoped_fbo(fbo);
  gl::ScopedMatrices scoped_matrices;
  gl::ScopedViewport scoped_viewport(ivec2(0), fbo->getSize());
  gl::setMatricesWindow(fbo->getSize());
  gl::clear(ColorA(0, 0, 0, 0));
  draw();
  set_texture(fbo->getColorTexture());
}

/////////////////////////////////////////////////
//
//  VideoProvider
//
/////////////////////////////////////////////////
/*
std::vector<ciWMFVideoPlayerRef> VideoProvider::players;

VideoProviderRef VideoProvider::create(std::string filename) {
  return std::make_shared<VideoProvider>(filename);
}

int VideoProvider::playerListIndex = 0;

VideoProvider::VideoProvider(std::string filename) {
  ready = false;
  playerIndex = VideoProvider::getPlayerIndex();
  setSource(filename);
}

void VideoProvider::setSource(std::string path) {
  source = path;
  VideoProvider::players[playerIndex]->loadMovie(ci::app::getAssetPath(path));
}

void VideoProvider::update() {
  ciWMFVideoPlayerRef & player = VideoProvider::players[playerIndex];
  player->update();
  player->_player->m_pEVRPresenter->lockSharedTexture();
  setTexture(player->getTexture());
  
  if (playing && player->isStopped() && !player->isLooping() && !player->isPaused()) {
    playing = false;
    mediaCompleteSignal.emit();
  }
}

vec2 VideoProvider::getSize() { 
  ciWMFVideoPlayerRef & player = VideoProvider::players[playerIndex];
  return vec2(player->getWidth(), player->getHeight());
}

bool VideoProvider::isReady() {
  PlayerState state = VideoProvider::players[playerIndex]->_player->GetState();
  return
  state == PlayerState::Started ||
  state == PlayerState::Stopped ||
  state == PlayerState::Paused;
}

void VideoProvider::startMedia(bool loop) {
  VideoProvider::players[playerIndex]->setLoop(loop);
  VideoProvider::players[playerIndex]->play();
  mediaIsLooping = loop;
  playing = true;
}

  /////////////////////////////////////////////////
  //
  //  WebProvider
  //
  /////////////////////////////////////////////////
WebProviderRef WebProvider::create(std::string url, ci::vec2 size, WebView * webView) {
  return std::make_shared<WebProvider>(url, size, webView);
}

WebProvider::WebProvider(std::string url, vec2 size, WebView * webView) : view(webView), viewSize(size) {
  view->SetTransparent(true);
  
  JSValue result = view->CreateGlobalJavascriptObject(WSLit("storyscreens"));
  
  if (result.IsObject()) {
    JSObject & jsObject = result.ToObject();
    jsObject.SetProperty(ToWebString("data"), AwesomiumSingleton::getPersistentStorage());
    
    dispatcher.Bind(jsObject, WSLit("cue_complete"), JSDelegate(this, &WebProvider::onCueComplete));
    
    dispatcher.Bind(jsObject, WSLit("write_data"), JSDelegate(this, &WebProvider::onPersistentStorageWrite));
    
    dispatcher.Bind(jsObject, WSLit("log"), JSDelegate(this, &WebProvider::onWebViewLog));
  }
  
  view->set_js_method_handler(&dispatcher);
  setSource(url);
}

WebProvider::~WebProvider() {
  view->Destroy();
}

void WebProvider::setSource(std::string url) {
  source = url;
  view->LoadURL(WebURL(WSLit(url.c_str())));
}

vec2 WebProvider::getSize() {
  return viewSize;
}

bool WebProvider::isReady() {
  return !view->IsLoading();
}

void WebProvider::onCueComplete(WebView* callingView, const JSArray& args) {
  mediaCompleteSignal.emit();
}

void WebProvider::onPersistentStorageWrite(WebView* callingView, const JSArray& args) {
  if (args.size()) {
    AwesomiumSingleton::updatePersistentStorage(args.At(0).ToString());
  }
}

void WebProvider::onWebViewLog(Awesomium::WebView* callingView, const Awesomium::JSArray & args) {
  if (args.size()) {
    CI_LOG_I(args.At(0).ToString());
  }
}

void WebProvider::update() {
  if (view) {
    BitmapSurface * surface = (BitmapSurface*)view->surface();
    if (surface && surface->is_dirty()) {
      setTexture(toTexture(surface));
    }
  }
}
*/
