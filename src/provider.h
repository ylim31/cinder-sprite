#pragma once

  // cinder
#include "cinder/gl/Texture.h"
#include "cinder/qtime/QuickTimeGl.h"


/////////////////////////////////////////////////
//
//  texture_provider
//  Base class for texture providers
//
/////////////////////////////////////////////////
class texture_provider {
public:
  //////////////////////////////////////////////////////
  // getters
  //////////////////////////////////////////////////////
  ci::signals::Signal<void()> & get_media_complete_signal();
  
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
  
protected:
  //////////////////////////////////////////////////////
  // properties
  //////////////////////////////////////////////////////
  ci::gl::TextureRef texture;
  bool ready;
  std::string source;
  bool texture_is_new;
  bool media_is_looping;
  ci::signals::Signal<void()> media_complete_signal;
  
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
  
  //////////////////////////////////////////////////////
  // getters
  //////////////////////////////////////////////////////
  ci::vec2 get_size() override;
  
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
  
  virtual bool is_ready() override;
  
  virtual void set_source(std::string path) override;
  
  virtual void update() override;
  
  ci::qtime::MovieGlRef get_movie() { return movie; }
  
protected:
  ci::qtime::MovieGlRef movie;
}; typedef video_provider::video_provider_ref video_provider_ref;


/*
/////////////////////////////////////////////////
//
//  VideoProvider
//
/////////////////////////////////////////////////
class VideoProvider : virtual public TextureProvider {
public:
    //////////////////////////////////////////////////////
    // static
    //////////////////////////////////////////////////////
  typedef std::shared_ptr<ciWMFVideoPlayer> ciWMFVideoPlayerRef;
  typedef std::shared_ptr<VideoProvider> VideoProviderRef;
  static VideoProviderRef create(std::string filename);
  static std::vector<ciWMFVideoPlayerRef> players;
  static int playerListIndex;
  
  static int getPlayerIndex() {
    int curr = playerListIndex;
    if (players.size() <= playerListIndex) {
      players.push_back(std::make_shared<ciWMFVideoPlayer>());
    }
    playerListIndex++;
    return curr;
  }
  
  static void VideoProvider::reset() {
    playerListIndex = 0;
    for (auto & player : players) {
      player->stop();
      player->loadMovie("");
    }
  }
  
  static void VideoProvider::pause() {
    for (auto & player : players) {
      if (player->isPlaying()) {
        player->pause();
      }
    }
  }
  
  static void VideoProvider::unpause() {
    for (auto & player : players) {
      if (player->isPaused()) {
        player->play();
      }
    }
  }
  
  static void VideoProvider::clear() {
    players.clear();
  }
  
    //////////////////////////////////////////////////////
    // ctr(s)
    //////////////////////////////////////////////////////
  VideoProvider(std::string filename);
  
    //////////////////////////////////////////////////////
    // getters
    //////////////////////////////////////////////////////
  ci::vec2 getSize() override;
  
  bool isReady() override;
  
    //////////////////////////////////////////////////////
    // setters
    //////////////////////////////////////////////////////
  void setSource(std::string path) override;
  
    //////////////////////////////////////////////////////
    // methods
    //////////////////////////////////////////////////////
  void startMedia(bool loop) override;
  
  void update() override;
  
private:
    //////////////////////////////////////////////////////
    // properties
    //////////////////////////////////////////////////////
  bool playing;
  int playerIndex;
};
typedef std::shared_ptr<ciWMFVideoPlayer> ciWMFVideoPlayerRef;
typedef VideoProvider::VideoProviderRef VideoProviderRef;

  /////////////////////////////////////////////////
  //
  //  WebProvider
  //
  /////////////////////////////////////////////////
class WebProvider : virtual public TextureProvider {
public:
    //////////////////////////////////////////////////////
    // static
    //////////////////////////////////////////////////////
  typedef std::shared_ptr<WebProvider> WebProviderRef;
  static WebProviderRef create(
                               std::string url, ci::vec2 size, Awesomium::WebView * webView);
  
    //////////////////////////////////////////////////////
    // ctr(s)
    //////////////////////////////////////////////////////
  WebProvider(
              std::string url,
              ci::vec2 size,
              Awesomium::WebView * webView);
  
  ~WebProvider();
  
    //////////////////////////////////////////////////////
    // properties
    //////////////////////////////////////////////////////
    //Awesomium::WebCore * core;
  Awesomium::WebView * view;
  MethodDispatcher dispatcher;
  ci::vec2 viewSize;
  
    //////////////////////////////////////////////////////
    // getters
    //////////////////////////////////////////////////////
  ci::vec2 getSize() override;
  
  bool isReady() override;
  
    //////////////////////////////////////////////////////
    // setters
    //////////////////////////////////////////////////////
  void setSource(std::string path) override;
  
    //////////////////////////////////////////////////////
    // methods
    //////////////////////////////////////////////////////
  void update() override;
  
private:
  void onCueComplete(Awesomium::WebView* callingView, const Awesomium::JSArray& args);
  
  void onPersistentStorageWrite(Awesomium::WebView* callingView, const Awesomium::JSArray& args);
  
  void onWebViewLog(Awesomium::WebView* callingView, const Awesomium::JSArray & args);
};
typedef WebProvider::WebProviderRef WebProviderRef;
*/
