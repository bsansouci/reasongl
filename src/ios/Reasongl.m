#import "Reasongl.h"
#import "GameKit/GameKit.h"
#import <OpenGLES/ES2/glext.h>

extern void reasonglMain(ReasonglViewController *this);
extern void reasonglWindowResize(void);
extern void reasonglUpdate(ReasonglViewController *this);
extern void reasonglTouchPress(double* touchPoints, int touchCount);
extern void reasonglTouchRelease(double* touchPoints, int touchCount);
extern void reasonglTouchDrag(double* touchPoints, int touchCount);

@interface ReasonglViewController () <GKGameCenterControllerDelegate>

@property (nonatomic, copy) NSString *leaderboardId;
@property (nonatomic, copy) void(^shouldGetPlayerHighScoreWhenAuthenticated)(int64_t scoreValue);

- (void)tearDownGL;

@end

@implementation ReasonglViewController {
    GKGameCenterViewController *_gameCenterController;
}

- (void)viewDidLoad
{
	[super viewDidLoad];

	self.view.multipleTouchEnabled = YES;
	self.viewSize = self.view.bounds.size;

    GKLocalPlayer *player = [GKLocalPlayer localPlayer];
    player.authenticateHandler = ^(UIViewController *viewController, NSError *error) {
        if (viewController) {
            [self presentViewController:viewController animated:YES completion:nil];
        } else if (error || !player.authenticated) {
            NSLog(@"Error authenticating: %@", error);
        } else {
            if (self.shouldGetPlayerHighScoreWhenAuthenticated) {
                [self getPlayerHighScore:self.shouldGetPlayerHighScoreWhenAuthenticated withLeaderboardId:self.leaderboardId];
                self.shouldGetPlayerHighScoreWhenAuthenticated = nil;
                self.leaderboardId = nil;
            }
        }
    };
    reasonglMain(self);
}

- (void)dealloc
{
	[super dealloc];
	[self tearDownGL];

	if ([EAGLContext currentContext] == self.context) {
		[EAGLContext setCurrentContext:nil];
	}
}

- (void)didReceiveMemoryWarning {
	[super didReceiveMemoryWarning];

	if ([self isViewLoaded] && ([[self view] window] == nil)) {
		self.view = nil;

		[self tearDownGL];

		if ([EAGLContext currentContext] == self.context) {
			[EAGLContext setCurrentContext:nil];
		}
		self.context = nil;
	}

	// Dispose of any resources that can be recreated.
}

- (BOOL)prefersStatusBarHidden {
	return YES;
}

- (void)tearDownGL {
	[EAGLContext setCurrentContext:self.context];
}

#pragma mark - Helpers

- (void)presentLeaderboardViewController:(NSString *)leaderboardId {
    _gameCenterController = [[GKGameCenterViewController alloc] init];
    if (!_gameCenterController) {
        NSLog(@"Couldn't create game center viewcontroller");
        return;
    }

    _gameCenterController.gameCenterDelegate = self;
    _gameCenterController.viewState = GKGameCenterViewControllerStateLeaderboards;
    _gameCenterController.leaderboardTimeScope = GKLeaderboardTimeScopeToday;
    _gameCenterController.leaderboardIdentifier = leaderboardId;
    [self presentViewController:_gameCenterController animated: YES completion:nil];
}

- (void)uploadScore:(int)scoreValue withLeaderboardId:(NSString *)leaderboardId {
    GKScore *score = [[GKScore alloc] initWithLeaderboardIdentifier:leaderboardId];
    score.value = scoreValue;
    [GKScore reportScores:@[score] withCompletionHandler:^(NSError *error) {
        if (error) {
            NSLog(@"Error uploading score: %@", error);
            return;
        }

        NSLog(@"Score %d successfully uploaded", scoreValue);
    }];
}

- (void)getPlayerHighScore:(void(^)(int64_t scoreValue))callback withLeaderboardId:(NSString *)leaderboardId {
    GKLocalPlayer *player = [GKLocalPlayer localPlayer];
    if (!player.authenticated) {
        self.shouldGetPlayerHighScoreWhenAuthenticated = callback;
        self.leaderboardId = leaderboardId;
        return;
    }

    GKLeaderboard *leaderboardRequest = [[GKLeaderboard alloc] initWithPlayers:@[player]];
    leaderboardRequest.identifier = leaderboardId;
    if (leaderboardRequest != nil) {
        [leaderboardRequest loadScoresWithCompletionHandler:^(NSArray *scores, NSError *error){
            if (error != nil) {
                NSLog(@"Couldn't retrieve scores for player %@", player);
                return;
            }

            callback(leaderboardRequest.localPlayerScore.value);
        }];
    }
}

#pragma mark - GLKView and GLKViewController delegate methods

- (void)viewWillTransitionToSize:(CGSize)size
       withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator
{
	self.viewSize = size;
	reasonglWindowResize();
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	[super touchesBegan:touches withEvent:event];
	double touchPoints[[touches count] * 3];
	int i = 0;
	for (UITouch* touchEvent in touches) {
		CGPoint locationInView = [touchEvent locationInView:self.view];
		touchPoints[i * 3 + 0] = [touchEvent hash];
		touchPoints[i * 3 + 1] = locationInView.x;
		touchPoints[i * 3 + 2] = locationInView.y;
		i++;
	}
	reasonglTouchPress(touchPoints, (int)[touches count]);
	// UITouch* touchEvent = [touches anyObject];
	// reasonglTouchPress(locationInView.x, locationInView.y);
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	[super touchesMoved:touches withEvent:event];
	// UITouch* touchEvent = [touches anyObject];
	// CGPoint locationInView = [touchEvent locationInView:self.view];
	// reasonglTouchDrag(locationInView.x, locationInView.y);
	double touchPoints[[touches count] * 3];
	int i = 0;
	for (UITouch* touchEvent in touches) {
		CGPoint locationInView = [touchEvent locationInView:self.view];
		touchPoints[i * 3 + 0] = [touchEvent hash];
		touchPoints[i * 3 + 1] = locationInView.x;
		touchPoints[i * 3 + 2] = locationInView.y;
		i++;
	}
	reasonglTouchDrag(touchPoints, (int)[touches count]);
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	[super touchesEnded:touches withEvent:event];
	// UITouch* touchEvent = [touches anyObject];
	// CGPoint locationInView = [touchEvent locationInView:self.view];
	// reasonglTouchRelease(locationInView.x, locationInView.y);
	double touchPoints[[touches count] * 3];
	int i = 0;
	for (UITouch* touchEvent in touches) {
		CGPoint locationInView = [touchEvent locationInView:self.view];
		touchPoints[i * 3 + 0] = [touchEvent hash];
		touchPoints[i * 3 + 1] = locationInView.x;
		touchPoints[i * 3 + 2] = locationInView.y;
		i++;
	}
	reasonglTouchRelease(touchPoints, (int)[touches count]);
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	[super touchesCancelled:touches withEvent:event];
	double touchPoints[[touches count] * 3];
	int i = 0;
	for (UITouch* touchEvent in touches) {
		CGPoint locationInView = [touchEvent locationInView:self.view];
		touchPoints[i * 3 + 0] = [touchEvent hash];
		touchPoints[i * 3 + 1] = locationInView.x;
		touchPoints[i * 3 + 2] = locationInView.y;
		i++;
	}
	reasonglTouchRelease(touchPoints, (int)[touches count]);
}

- (void)update {
    reasonglUpdate(self);
}

#pragma mark - GKGameCenterControllerDelegate

- (void)gameCenterViewControllerDidFinish:(GKGameCenterViewController *)gameCenterViewController {
    [_gameCenterController dismissViewControllerAnimated:YES completion:nil];
    _gameCenterController = nil;
}

@end
