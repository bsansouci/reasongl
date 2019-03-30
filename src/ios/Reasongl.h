#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>

@interface ReasonglViewController : GLKViewController
@property (strong, nonatomic) EAGLContext *context;
@property (nonatomic) CGSize viewSize;

- (void)presentLeaderboardViewController:(NSString *)leaderboardId;
- (void)uploadScore:(int)scoreValue withLeaderboardId:(NSString *)leaderboardId;
- (void)getPlayerHighScore:(void(^)(int64_t scoreValue))callback withLeaderboardId:(NSString *)leaderboardId;

@end
