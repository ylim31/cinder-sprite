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
  void draw() override;
  void loop();
  sprite_ref stars;
  sprite_ref sonic;
  float start;
  bool first;
};

void SpriteDemoApp::setup() {
  first = true;
  ci::app::setWindowSize(vec2(960, 540));
  
  stars = sprite::create(image_provider::create("stars.jpg"));
  stars->set_origin(sprite::origin_point::TopLeft);
  stars->set_alpha(0.3);
  
  sonic = sprite::create(image_provider::create("sonic.jpg"));
  sonic->set_origin(sprite::origin_point::Center);
  sonic->set_coordinates(getWindowCenter());
  
  loop();
}

void SpriteDemoApp::loop() {
  stars->move_to(
    vec2(0, getWindowHeight() - stars->get_bounds().getHeight()), 120.0f, 0.0f)->setFinishFn([=] {
    stars->move_to(
      vec2(0, 0), 120.0f, 0.0f)->setFinishFn(std::bind(&SpriteDemoApp::loop, this));
  });
}

void SpriteDemoApp::mouseDown(MouseEvent event)  {
  first = false;
  sonic->mask_reveal(sprite::mask_type::FromCenter, first ? 0.001 : 2.0)->setFinishFn([=] {
    sonic->mask_hide(sprite::mask_type::ToCenter, 2.0f);
  });
}

void SpriteDemoApp::draw() {
  gl::clear(Color(0, 0, 0));
  gl::color(Color::white());
  sonic->draw();
  ci::gl::ScopedBlendAdditive blend;
  stars->draw();
}

CINDER_APP(SpriteDemoApp, RendererGl)
