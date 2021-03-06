#include <mbgl/gl/context.hpp>
#include <mbgl/util/offscreen_texture.hpp>

#include <cassert>
#include <cstring>

namespace mbgl {

OffscreenTexture::OffscreenTexture(OffscreenTexture&&) = default;
OffscreenTexture& OffscreenTexture::operator=(OffscreenTexture&&) = default;

class OffscreenTexture::Impl {
public:
    Impl(gl::Context& context_, const Size size_)
        : context(context_), size(std::move(size_)) {
        assert(!size.isEmpty());
    }
    Impl(gl::Context& context_,
         const Size size_,
         gl::Renderbuffer<gl::RenderbufferType::DepthComponent>& depth_)
        : context(context_), size(std::move(size_)), depth(&depth_) {
        assert(!size.isEmpty());
    }

    void bind() {
        if (!framebuffer) {
            texture = context.createTexture(size, gl::TextureFormat::RGBA);
            if (depth) {
                framebuffer = context.createFramebuffer(*texture, *depth);
            } else {
                framebuffer = context.createFramebuffer(*texture);
            }
        } else {
            context.bindFramebuffer = framebuffer->framebuffer;
        }

        context.activeTextureUnit = 0;
        context.scissorTest = false;
        context.viewport = { 0, 0, size };
    }

    PremultipliedImage readStillImage() {
        return context.readFramebuffer<PremultipliedImage>(size);
    }

    gl::Texture& getTexture() {
        assert(texture);
        return *texture;
    }

    const Size& getSize() const {
        return size;
    }

private:
    gl::Context& context;
    const Size size;
    optional<gl::Framebuffer> framebuffer;
    optional<gl::Texture> texture;
    gl::Renderbuffer<gl::RenderbufferType::DepthComponent>* depth = nullptr;
};

OffscreenTexture::OffscreenTexture(gl::Context& context,
                                   const Size size)
    : impl(std::make_unique<Impl>(context, std::move(size))) {
    assert(!size.isEmpty());
}

OffscreenTexture::OffscreenTexture(gl::Context& context,
                                   const Size size,
                                   gl::Renderbuffer<gl::RenderbufferType::DepthComponent>& renderbuffer)
    : impl(std::make_unique<Impl>(context, std::move(size), renderbuffer)) {
    assert(!size.isEmpty());
}

OffscreenTexture::~OffscreenTexture() = default;

void OffscreenTexture::bind() {
    impl->bind();
}

PremultipliedImage OffscreenTexture::readStillImage() {
    return impl->readStillImage();
}

gl::Texture& OffscreenTexture::getTexture() {
    return impl->getTexture();
}

const Size& OffscreenTexture::getSize() const {
    return impl->getSize();
}

} // namespace mbgl
