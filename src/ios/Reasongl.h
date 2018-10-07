#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>

@interface ReasonglViewController : GLKViewController
@property (strong, nonatomic) EAGLContext *context;
@property (nonatomic) CGSize viewSize;
@end
