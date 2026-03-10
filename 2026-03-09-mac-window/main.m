#import <Cocoa/Cocoa.h>

#import <Cocoa/Cocoa.h>
#import <dlfcn.h>
#import <sys/stat.h>

typedef void (*update_func)(unsigned char*, int, int, int);

@interface BitmapView : NSView {
    NSBitmapImageRep *_bitmap;
    void *_lib_handle;
    update_func _update_ptr;
    time_t _last_mod;
    int _frame;
}
@end

@implementation BitmapView

- (void)reloadLibrary {
    struct stat attr;
    stat("liblogic.dylib", &attr);

    // Only reload if the file has a newer modification time
    if (attr.st_mtime > _last_mod) {
        if (_lib_handle) dlclose(_lib_handle);

        // Load the new library
        _lib_handle = dlopen("./liblogic.dylib", RTLD_NOW);
        if (_lib_handle) {
            _update_ptr = (update_func)dlsym(_lib_handle, "update_buffer");
            _last_mod = attr.st_mtime;
            NSLog(@"Library Reloaded!");
        }
    }
}

- (void)drawRect:(NSRect)dirtyRect {
    [self reloadLibrary]; // Check for updates every frame

    if (_update_ptr) {
        unsigned char *data = [_bitmap bitmapData];
        _update_ptr(data, [self bounds].size.width, [self bounds].size.height, _frame++);
    }

    [_bitmap drawInRect:[self bounds]];

    // Force a redraw as fast as possible for animation
    [self setNeedsDisplay:YES];
}



// Necessary to allow the view to receive keyboard events
- (BOOL)acceptsFirstResponder {
    return YES;
}

- (instancetype)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        // Create a 32-bit RGBA bitmap buffer
        _bitmap = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
            pixelsWide:frame.size.width
            pixelsHigh:frame.size.height
            bitsPerSample:8
            samplesPerPixel:4
            hasAlpha:YES
            isPlanar:NO
            colorSpaceName:NSDeviceRGBColorSpace
            bytesPerRow:frame.size.width * 4
            bitsPerPixel:32];

        // Fill the buffer with some initial data (e.g., a purple tint)
        unsigned char *data = [_bitmap bitmapData];
        for (int i = 0; i < frame.size.width * frame.size.height * 4; i += 4) {
            data[i+0] = 150; // R
            data[i+1] = 50;  // G
            data[i+2] = 250; // B
            data[i+3] = 255; // A
        }
    }
    return self;
}

// Handle Key Presses
- (void)keyDown:(NSEvent *)event {
    NSString *characters = [event charactersIgnoringModifiers];
    unichar keyChar = [characters characterAtIndex:0];

    NSLog(@"Key Pressed: %C", keyChar);

    // Example: Change buffer color based on key
    unsigned char *data = [_bitmap bitmapData];
    unsigned char r = 0, g = 0, b = 0;

    if (keyChar == 'r') r = 255;
    else if (keyChar == 'g') g = 255;
    else if (keyChar == 'b') b = 255;
    else { r = 150; g = 50; b = 250; } // Default Purple

    for (int i = 0; i < [self bounds].size.width * [self bounds].size.height * 4; i += 4) {
        data[i+0] = r; data[i+1] = g; data[i+2] = b; data[i+3] = 255;
    }

    // Tell the system the view needs to be redrawn
    [self setNeedsDisplay:YES];
}

@end

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

        // Setup Window
        NSRect rect = NSMakeRect(0, 0, 800, 600);
        NSWindow *window = [[NSWindow alloc] initWithContentRect:rect
            styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable)
            backing:NSBackingStoreBuffered
            defer:NO];

        [window setTitle:@"Cocoa Bitmap Buffer"];

        BitmapView *view = [[BitmapView alloc] initWithFrame:rect];
        [window setContentView:view];
        [window makeKeyAndOrderFront:nil];

        // Ensure the view is ready to capture keys immediately
        [window makeFirstResponder:view];

        [NSApp activateIgnoringOtherApps:YES];
        [NSApp run];
    }
    return 0;
}
