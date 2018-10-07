#import <UIKit/UIKit.h>
#import "AppDelegate.h"
#import "caml/callback.h"

int main(int argc, char * argv[]) {
    @autoreleasepool {
		// Initialize OCaml.
        caml_main(argv);
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
