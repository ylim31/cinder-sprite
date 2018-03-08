#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

// cinder-sprite
#include "sprite.h"
#include "provider.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class custom_graphics : public graphics_provider {
  public:
    custom_graphics(vec2 size);
    void draw() override;
    Anim<vec2> p;
    vec2 target;
};

custom_graphics::custom_graphics(vec2 size) : graphics_provider(size) {
  p = vec2(256, 256);
  target = vec2(256, 256);
  getWindow()->getSignalMouseMove().connect([=](MouseEvent e) {
    target = e.getPos();
  });
}

void custom_graphics::draw() {
  p() += (target - p()) * 0.2f;
  gl::drawSolidCircle(p, 32);
}

class GraphicsSpriteDemoApp : public App {
  public:
	void setup() override;
	void update() override;
	void draw() override;
  void loop();
 
  sprite_ref sp;
  std::shared_ptr<custom_graphics> graphics;
};

void GraphicsSpriteDemoApp::setup() {
  graphics = std::make_shared<custom_graphics>(vec2(512, 512));
  sp = sprite::create(graphics);
  loop();
}

void GraphicsSpriteDemoApp::loop() {
  sp->alpha_to(0.0f, 3.0f)->setFinishFn([=] {
    sp->alpha_to(1.0f, 3.0f)->setFinishFn([=] {
      loop();
    });
  });
}

void GraphicsSpriteDemoApp::update() {
  graphics->update();
}

void GraphicsSpriteDemoApp::draw() {
	gl::clear(Color(0, 0, 0));
  sp->draw();
}

CINDER_APP(GraphicsSpriteDemoApp, RendererGl(RendererGl::Options().msaa(16)))
