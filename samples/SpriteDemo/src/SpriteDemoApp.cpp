#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Timeline.h"
#include "cinder/Rand.h"
#include "cinder/Log.h"

#include "sprite.h"
#include "provider.h"


using namespace ci;
using namespace ci::app;
using namespace std;

class SpriteDemoApp : public App {
public:
  void setup() override;
  void mouseDown( MouseEvent event ) override;
  void update() override;
  void draw() override;
  sprite_ref sprite;
  TimelineRef animator;
  float start;
  float time;
};

void SpriteDemoApp::setup() {
  animator = Timeline::create();
  sprite = sprite::create(image_provider::create("sonic.jpg"));
  sprite->update();
}

void SpriteDemoApp::mouseDown(MouseEvent event)  {
  time = start = getElapsedSeconds();
  int x = ci::randFloat() * getWindowWidth();
  int y = ci::randFloat() * getWindowHeight();
  sprite->move_to(animator, vec2(x, y), 2.0f);
  sprite->mask_hide(animator, "right-to-left", 2.0f, 2.0f);
}

void SpriteDemoApp::update() {
  //animator->step(1/(getAverageFps() * 1000));
  animator->stepTo(getElapsedSeconds(), true);
}

void SpriteDemoApp::draw() {
  gl::clear(Color(0, 0, 0));
  gl::color(Color::white());
  sprite->draw();
}

CINDER_APP(SpriteDemoApp, RendererGl)
