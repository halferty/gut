/**
 * @file metal_demo.mm
 * @brief macOS + Metal demo for Gut UI Toolkit
 */

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <CoreVideo/CoreVideo.h>

#include "gut_single.h"

// Include the Metal backend implementation
#include "metal_backend.mm"

#include <memory>
#include <chrono>

// Shared UI code
#include "demo_ui.h"

@interface MetalView : NSView {
    std::unique_ptr<gut::Context> _context;
    std::vector<gut::Ref<gut::FloatAnimation>> _animations;
    std::chrono::high_resolution_clock::time_point _lastTime;
}
@property (nonatomic, strong) CAMetalLayer* metalLayer;
@property (nonatomic) CVDisplayLinkRef displayLink;
@end

@implementation MetalView

- (instancetype)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:frameRect];
    if (self) {
        _lastTime = std::chrono::high_resolution_clock::now();
        [self setupMetal];
        [self setupGut];
        [self setupDisplayLink];
    }
    return self;
}

- (void)dealloc {
    if (_displayLink) {
        CVDisplayLinkStop(_displayLink);
        CVDisplayLinkRelease(_displayLink);
    }
}

- (void)setupMetal {
    self.wantsLayer = YES;
    self.metalLayer = [CAMetalLayer layer];
    self.metalLayer.device = MTLCreateSystemDefaultDevice();
    self.metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    self.metalLayer.framebufferOnly = NO;
    self.layer = self.metalLayer;
}

- (void)setupGut {
    auto backend = std::make_unique<gut::MetalRenderBackend>(self.metalLayer);
    _context = std::make_unique<gut::Context>(std::move(backend));

    // Clipboard callbacks (macOS)
    _context->setOnGetClipboardText([]() -> gut::String {
        NSPasteboard* pb = [NSPasteboard generalPasteboard];
        NSString* str = [pb stringForType:NSPasteboardTypeString];
        if (str) {
            return gut::String([str UTF8String]);
        }
        return {};
    });
    _context->setOnSetClipboardText([](const gut::String& text) {
        NSPasteboard* pb = [NSPasteboard generalPasteboard];
        [pb clearContents];
        [pb setString:[NSString stringWithUTF8String:text.c_str()]
              forType:NSPasteboardTypeString];
    });

    // Load system fonts — multiple weights for font weight/style demo
    {
        NSArray* fontPaths = @[
            @"/System/Library/Fonts/Supplemental/Arial.ttf",
            @"/System/Library/Fonts/Supplemental/Arial Bold.ttf",
            @"/System/Library/Fonts/Supplemental/Arial Italic.ttf",
            @"/System/Library/Fonts/Supplemental/Arial Bold Italic.ttf",
        ];
        for (NSString* path in fontPaths) {
            NSData* fontData = [NSData dataWithContentsOfFile:path];
            if (fontData) {
                _context->loadFont(
                    static_cast<const gut::u8*>(fontData.bytes),
                    fontData.length);
            }
        }
    }

    // Build shared UI
    NSSize size = self.bounds.size;
    demo::DemoConfig cfg;
    cfg.width = static_cast<gut::u32>(size.width);
    cfg.height = static_cast<gut::u32>(size.height);
    cfg.backendName = "Metal";
    cfg.backendShort = "MTL";
    cfg.shaderModel = "MSL 2.0";

    auto result = demo::buildDemoUI(*_context, cfg);
    _context->setRoot(result.root);
    _animations = std::move(result.animations);
}

static CVReturn DisplayLinkCallback(
    CVDisplayLinkRef displayLink,
    const CVTimeStamp* now,
    const CVTimeStamp* outputTime,
    CVOptionFlags flagsIn,
    CVOptionFlags* flagsOut,
    void* displayLinkContext)
{
    @autoreleasepool {
        MetalView* view = (__bridge MetalView*)displayLinkContext;
        [view render];
    }
    return kCVReturnSuccess;
}

- (void)setupDisplayLink {
    CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
    CVDisplayLinkSetOutputCallback(_displayLink, &DisplayLinkCallback, (__bridge void*)self);
    CVDisplayLinkStart(_displayLink);
}

- (void)render {
    if (!_context) return;

    NSSize size = self.bounds.size;
    CGFloat scale = self.window.backingScaleFactor;

    self.metalLayer.drawableSize = CGSizeMake(size.width * scale, size.height * scale);

    auto now = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float, std::milli>(now - _lastTime).count();
    _lastTime = now;

    _context->update(dt);
    _context->render(size.width, size.height, scale);
}

- (void)setFrameSize:(NSSize)newSize {
    [super setFrameSize:newSize];
    CGFloat scale = self.window.backingScaleFactor;
    self.metalLayer.drawableSize = CGSizeMake(newSize.width * scale, newSize.height * scale);
}

// ============================================================================
// Input handling
// ============================================================================

- (void)mouseDown:(NSEvent*)event {
    NSPoint location = [self convertPoint:event.locationInWindow fromView:nil];
    location.y = self.bounds.size.height - location.y;
    _context->processMouseMove(location.x, location.y);
    _context->processMouseButton(gut::MouseButton::Left, true);
}

- (void)mouseUp:(NSEvent*)event {
    NSPoint location = [self convertPoint:event.locationInWindow fromView:nil];
    location.y = self.bounds.size.height - location.y;
    _context->processMouseMove(location.x, location.y);
    _context->processMouseButton(gut::MouseButton::Left, false);
}

- (void)rightMouseDown:(NSEvent*)event {
    NSPoint location = [self convertPoint:event.locationInWindow fromView:nil];
    location.y = self.bounds.size.height - location.y;
    _context->processMouseMove(location.x, location.y);
    _context->processMouseButton(gut::MouseButton::Right, true);
}

- (void)rightMouseUp:(NSEvent*)event {
    NSPoint location = [self convertPoint:event.locationInWindow fromView:nil];
    location.y = self.bounds.size.height - location.y;
    _context->processMouseMove(location.x, location.y);
    _context->processMouseButton(gut::MouseButton::Right, false);
}

- (void)mouseMoved:(NSEvent*)event {
    NSPoint location = [self convertPoint:event.locationInWindow fromView:nil];
    location.y = self.bounds.size.height - location.y;
    _context->processMouseMove(location.x, location.y);
}

- (void)mouseDragged:(NSEvent*)event {
    [self mouseMoved:event];
}

- (void)rightMouseDragged:(NSEvent*)event {
    [self mouseMoved:event];
}

- (void)scrollWheel:(NSEvent*)event {
    _context->processMouseWheel(event.scrollingDeltaX, event.scrollingDeltaY);
}

- (void)keyDown:(NSEvent*)event {
    gut::ModifierKeys mods = gut::ModifierKeys::None;
    NSEventModifierFlags flags = event.modifierFlags;
    if (flags & NSEventModifierFlagShift)   mods = mods | gut::ModifierKeys::Shift;
    if (flags & NSEventModifierFlagCommand) mods = mods | gut::ModifierKeys::Control;
    if (flags & NSEventModifierFlagOption)  mods = mods | gut::ModifierKeys::Alt;

    _context->processKey(static_cast<gut::Key>(event.keyCode), true, mods);

    // Text input
    NSString* chars = event.characters;
    if (chars.length > 0) {
        unichar ch = [chars characterAtIndex:0];
        if (ch >= 32 && !(flags & NSEventModifierFlagCommand)) {
            _context->processTextInput(static_cast<char32_t>(ch));
        }
    }
}

- (void)keyUp:(NSEvent*)event {
    _context->processKey(static_cast<gut::Key>(event.keyCode), false);
}

- (void)updateTrackingAreas {
    [super updateTrackingAreas];
    for (NSTrackingArea* area in self.trackingAreas) {
        [self removeTrackingArea:area];
    }
    NSTrackingArea* trackingArea = [[NSTrackingArea alloc]
        initWithRect:self.bounds
        options:(NSTrackingMouseMoved | NSTrackingActiveInKeyWindow |
                 NSTrackingInVisibleRect | NSTrackingMouseEnteredAndExited)
        owner:self
        userInfo:nil];
    [self addTrackingArea:trackingArea];
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

@end

// ============================================================================
// App delegate & entry point
// ============================================================================

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (nonatomic, strong) NSWindow* window;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notification {
    gut::initialize();

    NSRect frame = NSMakeRect(0, 0, 1100, 720);

    self.window = [[NSWindow alloc]
        initWithContentRect:frame
        styleMask:(NSWindowStyleMaskTitled |
                  NSWindowStyleMaskClosable |
                  NSWindowStyleMaskMiniaturizable |
                  NSWindowStyleMaskResizable)
        backing:NSBackingStoreBuffered
        defer:NO];

    self.window.title = @"Gut \u2014 Metal Demo";
    [self.window center];

    MetalView* view = [[MetalView alloc] initWithFrame:frame];
    self.window.contentView = view;

    [self.window makeKeyAndOrderFront:nil];
    [self.window makeFirstResponder:view];
}

- (void)applicationWillTerminate:(NSNotification*)notification {
    gut::shutdown();
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
    return YES;
}

@end

int main(int argc, const char* argv[]) {
    @autoreleasepool {
        NSApplication* app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];

        AppDelegate* delegate = [[AppDelegate alloc] init];
        app.delegate = delegate;

        [app activateIgnoringOtherApps:YES];
        [app run];
    }
    return 0;
}
