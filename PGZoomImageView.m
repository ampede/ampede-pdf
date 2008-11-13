/*
 *  PGZoomImageView.m
 *  graphviz
 *
 *  Created by Glen Low on Thu Jan 22 2004.
 *  Copyright (c) 2004, Pixelglow Software. All rights reserved.
 *  http://www.pixelglow.com/graphviz/
 *  graphviz@pixelglow.com
 *
 *  Redistribution and use in source and binary forms, with or without modification, are permitted
 *  provided that the following conditions are met:
 *  * Redistributions of source code must retain the above copyright notice, this list of conditions
 *    and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions
 *    and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *  * Neither the name of Pixelglow Software nor the names of its contributors may be used to endorse or
 *    promote products derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#import "PGZoomImageView.h"

static NSCursor* closedHandCursor_ = nil;
static NSCursor* openHandCursor_ = nil;
static Class shadowClass_ = nil;

@implementation PGZoomImageView

+ (void) initialize
	{
		Class cursorClass = [NSCursor class];
		if ([cursorClass respondsToSelector: @selector (closedHandCursor)])
			closedHandCursor_ = [cursorClass closedHandCursor];
		if ([cursorClass respondsToSelector: @selector (openHandCursor)])
			openHandCursor_ = [cursorClass openHandCursor];	
		shadowClass_ = NSClassFromString (@"NSShadow");
	}

- (id) initWithFrame: (NSRect) frame
	{
		if ((self = [super initWithFrame: frame]))
			{
				zoom_ = 1.0;
				targetX_ = targetY_ = 0.5;
				doTarget_ = YES;
			}
			
		return self;	
	}

- (BOOL) retarget
	{
		NSRect visibleRect = [[self enclosingScrollView] documentVisibleRect];
		
		
		// move the image so that the target point is in the center of the visible rect
		NSPoint oldOrigin = imaging_.origin;
		imaging_.origin = NSMakePoint (
			NSMidX (visibleRect) - imaging_.size.width * targetX_,
			NSMidY (visibleRect) - imaging_.size.height * targetY_);
		
		// the new frame must enclose both the visible rect and the image
		NSRect frame = NSUnionRect (visibleRect, imaging_);
		
		// normalize values so that frame origin is 0, 0
		imaging_.origin.x -= frame.origin.x;
		imaging_.origin.y -= frame.origin.y;
		visibleRect.origin.x -= frame.origin.x;
		visibleRect.origin.y -= frame.origin.y;
				
		// adjust frame and scroll
		doTarget_ = NO;
		NSSize oldSize = [self frame].size;
		[self setFrameSize: frame.size];
		[self scrollPoint: visibleRect.origin];
		doTarget_ = YES;
		
		return !NSEqualPoints (oldOrigin, imaging_.origin) || !NSEqualSizes (oldSize, frame.size);
	}

- (void) adjustFrame
	{
		if (image_)
			{
				NSSize size = [image_ size];
				imaging_.size = NSMakeSize (size.width * zoom_, size.height * zoom_);
				
				[self retarget];
			}
	}
	
- (BOOL) isOpaque
	{
		return YES;
	}

- (void) drawRect: (NSRect) rect
	{
		// clear rect
		[[NSColor controlBackgroundColor] set];
		NSRectFill (rect);

		// draw shadow only if we are zoomed small enough to see non-image portion
		if (shadowClass_ && !NSEqualRects ([self frame], imaging_))
			{
				NSShadow* shadow = [[shadowClass_ alloc] init];
				[shadow setShadowColor: [NSColor controlShadowColor]];
				[shadow setShadowOffset: NSMakeSize (0.0, -1.0 - zoom_ / 2.0)];
				[shadow setShadowBlurRadius: 2.0 + zoom_];
				[shadow set];
				[[NSColor controlBackgroundColor] set];
				NSRectFill (imaging_);
				[shadow release];
			}

		NSRect imageBounds;
		imageBounds.origin.x = imageBounds.origin.y = 0.0;
		imageBounds.size = [image_ size];
		[image_ drawInRect: imaging_ fromRect: imageBounds
			operation: NSCompositeSourceOver fraction: 1.0];
	}
	
- (BOOL) acceptsFirstResponder
	{
		return YES;
	}
	
- (void) keyDown: (NSEvent*) event
	{
		[self interpretKeyEvents: [NSArray arrayWithObject: event]];
	}
	
- (void) mouseDown: (NSEvent*) event
	{
		// NSRect visible = [self visibleRect];
		float oldTargetX = targetX_;
		float oldTargetY = targetY_;
		NSPoint oldLocation = [event locationInWindow];
//		[[self enclosingScrollView] setAutohidesScrollers: NO];
		
//		if (!NSEqualRects (visible, [self frame]))
			{
				NSPoint location = [event locationInWindow];
		//		NSPoint scroll = NSMakePoint (visible.origin.x + location.x, visible.origin.y + location.y);
				
				[closedHandCursor_ push];
				while (true)
					{
						event = [[self window] nextEventMatchingMask: NSLeftMouseDraggedMask | NSLeftMouseUpMask];
						switch ([event type])
							{
								case NSLeftMouseDragged:
									location = [event locationInWindow];
									
									targetX_ = oldTargetX + (oldLocation.x - location.x) / imaging_.size.width;
									targetY_ = oldTargetY + (oldLocation.y - location.y) / imaging_.size.height;
									if ([self retarget])
										[self setNeedsDisplay: YES];
									
								//	[self scrollPoint: NSMakePoint (scroll.x - location.x, scroll.y - location.y)];
									break;
									
								case NSLeftMouseUp:
									[closedHandCursor_ pop];
						//			[[self enclosingScrollView] setAutohidesScrollers: YES];
									return;
									
								default:
									break;
							}
					}
			}
	}
	
- (void) moveLeft: (id) sender
	{
		NSRect visible = [self visibleRect];
		visible.origin.x -= [[self enclosingScrollView] horizontalLineScroll];
		[self scrollPoint: visible.origin];
	}

- (void) moveRight: (id) sender
	{
		NSRect visible = [self visibleRect];
		visible.origin.x += [[self enclosingScrollView] horizontalLineScroll];
		[self scrollPoint: visible.origin];
	}
	
- (void) moveUp: (id) sender
	{
		NSRect visible = [self visibleRect];
		visible.origin.y += [[self enclosingScrollView] verticalLineScroll];
		[self scrollPoint: visible.origin];
	}

- (void) moveDown: (id) sender
	{
		NSRect visible = [self visibleRect];
		visible.origin.y -= [[self enclosingScrollView] verticalLineScroll];
		[self scrollPoint: visible.origin];
	}
	
- (void) resetCursorRects
	{
		if (openHandCursor_)
			[self addCursorRect: [self visibleRect] cursor: openHandCursor_];
	}
	
- (void) scrollPageUp: (id) sender
	{
		[[self enclosingScrollView] pageUp: sender];
	}

- (void) scrollPageDown: (id) sender
	{
		[[self enclosingScrollView] pageDown: sender];
	}

- (void) scrollToBeginningOfDocument: (id) sender
	{
		NSRect visible = [self visibleRect];
		NSRect frame = [self frame];
		
		visible.origin.y = frame.origin.y + frame.size.height - visible.size.height;
		[self scrollPoint: visible.origin];
	}
	
- (void) scrollToEndOfDocument: (id) sender
	{
		NSRect visible = [self visibleRect];
		NSRect frame = [self frame];
		
		visible.origin.y = frame.origin.y;
		[self scrollPoint: visible.origin];
	}

- (NSImage*) image
	{
		return image_;
	}
	
- (void) setImage: (NSImage*) image
	{
		if (image != image_)
			{
				[image_ release];
				image_ = [image retain];
				
				[self adjustFrame];
				[self setNeedsDisplay: YES];
			}
	}

- (double) zoom
	{
		return zoom_;
	}
	
- (void) setZoom: (double) zoom
	{
		zoom_ = zoom;
		[self adjustFrame];
		[self setNeedsDisplay: YES];
	}
	
- (void) superviewFrameDidChange: (NSNotification*) notification
	{
		[self retarget];
	}
	
- (void) viewWillMoveToSuperview: (NSView*) newSuperview
	{
		NSNotificationCenter* defaultCenter = [NSNotificationCenter defaultCenter];
		
		NSView* oldSuperview = [self superview];
		if (oldSuperview)
			[defaultCenter
				removeObserver: self
				name: NSViewFrameDidChangeNotification
				object: oldSuperview];
		if (newSuperview)
			[defaultCenter
				addObserver: self
				selector: @selector (superviewFrameDidChange:)
				name: NSViewFrameDidChangeNotification
				object: newSuperview];
	}
	
- (NSRect) adjustScroll: (NSRect) proposedVisibleRect
	{
		if (doTarget_)
			{
				targetX_ = (NSMidX (proposedVisibleRect) - imaging_.origin.x) / imaging_.size.width;
				targetY_ = (NSMidY (proposedVisibleRect) - imaging_.origin.y) / imaging_.size.height;
			}
		return proposedVisibleRect;
	}

	
@end
