#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"
#include "sprite.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class VideoSpriteDemoApp : public App {
  public:
	void setup() override;
	void update() override;
	void draw() override;
  void fileDrop(FileDropEvent e) override;
  sprite_ref sprite;
  video_provider_ref video;
};

void VideoSpriteDemoApp::setup() {
  video = video_provider::create();
  sprite = sprite::create(video);
}

void VideoSpriteDemoApp::update() {
  sprite->get_provider()->update();
}

void VideoSpriteDemoApp::fileDrop(FileDropEvent e) {
  try {
    if(e.getNumFiles() > 0) {
      ci::fs::path p = e.getFile(0);
      sprite->get_provider()->set_source(p.string());
      if(video->get_movie()) {
        video->get_movie()->setLoop(true);
      }
    }
  } catch(exception e) {
    CI_LOG_E("Error loading video:" << e.what());
  }
}

void VideoSpriteDemoApp::draw() {
	gl::clear(Color(0, 0, 0));
  if(sprite->get_provider()->is_ready()) {
    sprite->draw();
  } else {
    gl::color(Color::white());
    gl::drawString("Drop a video file here", vec2(30, 30));
  }
  
}

CINDER_APP( VideoSpriteDemoApp, RendererGl )
