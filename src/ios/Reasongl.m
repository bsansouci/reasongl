#import "Reasongl.h"
#import <OpenGLES/ES2/glext.h>

extern void reasonglMain(ReasonglViewController *this);
extern void reasonglWindowResize(void);
extern void reasonglUpdate(ReasonglViewController *this);
extern void reasonglTouchPress(double* touchPoints, int touchCount);
extern void reasonglTouchRelease(double* touchPoints, int touchCount);
extern void reasonglTouchDrag(double* touchPoints, int touchCount);

@interface ReasonglViewController ()

- (void)tearDownGL;

@end

@implementation ReasonglViewController

- (void)viewDidLoad
{
	[super viewDidLoad];
	self.view.multipleTouchEnabled = YES;
	self.viewSize = self.view.bounds.size;
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

@end
