#include "platform/x11/window.hpp"

#include "cpu_renderer/framebuffer.hpp"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <cstdlib>
#include <memory>
#include <new>
#include <stdexcept>

namespace cpu_renderer::x11 {
namespace {

struct ImageDeleter {
    void operator()(XImage* image) const noexcept { XDestroyImage(image); }
};

using Image = std::unique_ptr<XImage, ImageDeleter>;

class Channel {
public:
    explicit Channel(unsigned long mask) : mask_(mask)
    {
        if (mask == 0) {
            throw std::runtime_error("The X11 TrueColor visual has an empty channel mask");
        }
        while ((mask & 1UL) == 0) {
            ++shift_;
            mask >>= 1;
        }
        max_ = mask;
    }

    unsigned long pack(std::uint8_t value) const noexcept
    {
        const auto scaled = (static_cast<std::uint64_t>(value) * max_ + 127) / 255;
        return (static_cast<unsigned long>(scaled) << shift_) & mask_;
    }

private:
    unsigned long mask_;
    unsigned long max_;
    unsigned int shift_ = 0;
};

Image makeImage(Display* display, Visual* visual, int depth, const Framebuffer& frame)
{
    Image image{XCreateImage(display, visual, depth, ZPixmap, 0, nullptr,
                             frame.width(), frame.height(), 32, 0)};
    if (!image) {
        throw std::runtime_error("Could not create the X11 image");
    }

    // Xlib computes the stride and XDestroyImage owns the allocated data.
    image->data = static_cast<char*>(std::calloc(image->bytes_per_line, frame.height()));
    if (!image->data) {
        throw std::bad_alloc{};
    }

    const Channel red{image->red_mask};
    const Channel green{image->green_mask};
    const Channel blue{image->blue_mask};
    for (int y = 0; y < frame.height(); ++y) {
        for (int x = 0; x < frame.width(); ++x) {
            const auto [r, g, b] = frame.pixel(x, y);
            XPutPixel(image.get(), x, y, red.pack(r) | green.pack(g) | blue.pack(b));
        }
    }
    return image;
}

} // namespace

void runWindow()
{
    // Closing the connection also releases the window and other server resources.
    std::unique_ptr<Display, decltype(&XCloseDisplay)> connection{
        XOpenDisplay(nullptr), &XCloseDisplay
    };
    if (!connection) {
        throw std::runtime_error("Cannot open the X display. Run from an X11 or XWayland session");
    }

    auto* display = connection.get();
    const int screen = DefaultScreen(display);
    auto* visual = DefaultVisual(display, screen);
    if (visual->c_class != TrueColor) {
        throw std::runtime_error("The default X11 visual must support TrueColor");
    }
    const int depth = DefaultDepth(display, screen);

    int width = 800;
    int height = 480;
    const Window window = XCreateSimpleWindow(display, RootWindow(display, screen),
                                              0, 0, width, height, 0, 0,
                                              BlackPixel(display, screen));
    XStoreName(display, window, "CPU Renderer - Gradient");
    XSelectInput(display, window, ExposureMask | StructureNotifyMask);
    const Atom protocols = XInternAtom(display, "WM_PROTOCOLS", False);
    Atom close = XInternAtom(display, "WM_DELETE_WINDOW", False);
    if (!XSetWMProtocols(display, window, &close, 1)) {
        throw std::runtime_error("Could not register the X11 window close protocol");
    }

    Image image;
    const auto present = [&] {
        if (!image) {
            Framebuffer frame{width, height};
            drawGradient(frame);
            image = makeImage(display, visual, depth, frame);
        }
        XPutImage(display, window, DefaultGC(display, screen), image.get(),
                  0, 0, 0, 0, width, height);
        XFlush(display);
    };

    XMapWindow(display, window);
    for (;;) {
        XEvent event{};
        XNextEvent(display, &event);
        switch (event.type) {
        case Expose:
            if (event.xexpose.count == 0) {
                present();
            }
            break;
        case ConfigureNotify:
            if (width != event.xconfigure.width || height != event.xconfigure.height) {
                width = event.xconfigure.width;
                height = event.xconfigure.height;
                image.reset();
                present();
            }
            break;
        case ClientMessage:
            if (event.xclient.message_type == protocols && event.xclient.format == 32 &&
                static_cast<Atom>(event.xclient.data.l[0]) == close) {
                return;
            }
            break;
        case DestroyNotify:
            return;
        }
    }
}

} // namespace cpu_renderer::x11
