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
    void update() override;
    Anim<vec2> p;
    vec2 target;
};

custom_graphics::custom_graphics(vec2 size) : graphics_provider(size) {
  p = vec2(256, 256);
  getWindow()->getSignalMouseDown().connect([=](MouseEvent e) {
    target = e.getPos();
  });
}

void custom_graphics::update() {
  p() += (target - p()) * 0.2f;
  gl::ScopedMatrices scoped_matrices;
  gl::ScopedFramebuffer scoped_fbo(fbo);
  gl::ScopedViewport scoped_viewport(ivec2(0), fbo->getSize());
  gl::setMatricesWindow(fbo->getSize());
  gl::clear(ColorA(1, 0, 0, 1));
  gl::drawSolidCircle(p, 32);
  set_texture(fbo->getColorTexture());
}

class GraphicsSpriteDemoApp : public App {
  public:
	void setup() override;
	void update() override;
	void draw() override;
  void loop();
 
  sprite_ref graphics;
  TimelineRef animator;
};

void GraphicsSpriteDemoApp::setup() {
  animator = Timeline::create();
  graphics = sprite::create(
   std::make_shared<custom_graphics>(vec2(512, 512)));
  loop();
}

void GraphicsSpriteDemoApp::loop() {
  graphics->alpha_to(animator, 0.0f, 3.0f)->setFinishFn([=] {
    graphics->alpha_to(animator, 1.0f, 3.0f)->setFinishFn([=] {
      loop();
    });
  });
}

void GraphicsSpriteDemoApp::update() {
  animator->stepTo(getElapsedSeconds(), true);
  graphics->update();
}

void GraphicsSpriteDemoApp::draw() {
	gl::clear(Color(0, 0, 0));
  graphics->draw();
}

CINDER_APP(GraphicsSpriteDemoApp, RendererGl(RendererGl::Options().msaa(16)))
