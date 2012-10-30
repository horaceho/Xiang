//
//  XiangViewController.m
//  Xiang
//
//  Created by Horace Ho on 2012/10/29.
//  Copyright (c) 2012 Horace Ho. All rights reserved.
//

#import "engine.h"
#import "XiangViewController.h"

@interface XiangViewController ()
@property (nonatomic, strong) IBOutlet UITextView *textView;
@end

@implementation XiangViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    NSString * bookPath = [[NSBundle mainBundle] pathForResource:@"OPENBOOK" ofType:@"DAT"];
    engineInit([bookPath UTF8String]);
}

- (void)printBoard
{
    for (int rank = RankTop; rank <= RankBottom; rank++) {
        for (int file = FileLeft; file <= FileRight; file++) {
            char piece = engineGetPieceAt(file, rank);
            self.textView.text = [self.textView.text stringByAppendingString:[NSString stringWithFormat:@"%c ", piece]];
        }
        self.textView.text = [self.textView.text stringByAppendingString:@"\n"];
    }
    self.textView.text = [self.textView.text stringByAppendingString:@"\n"];
}

- (void)demoEngineUsage
{
    // setup engine with a FEN string
    NSString *openStr = @"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w";
    engineSetFEN([openStr UTF8String]);

    // print current board position in engine
    [self printBoard];

    // setup engine with an existing game
    self.textView.text = [self.textView.text stringByAppendingString:@"天衣无缝\n"];
    NSString *someStr = @"3akab2/1C6c/N3b4/9/1N7/9/9/C8/n4p3/rc2K1p2 w - - 0 1";
    engineSetFEN([someStr UTF8String]);
    [self printBoard];
    
    // start engine AI with 30 seconds
    unsigned long bestMove = engineThink(30);
    char string[sizeof(unsigned long)+1];
    if (bestMove > 0) {
        sprintf(string, "%.4s", (const char *) &bestMove);
    } else {
        strncpy(string, "none", sizeof(unsigned long));
    }
    string[sizeof(unsigned long)] = '\0';
        
    self.textView.text = [self.textView.text stringByAppendingString:@"最佳着法: "];
    self.textView.text = [self.textView.text stringByAppendingString:[NSString stringWithFormat:@"%s", string]];
    
    [self.textView scrollRangeToVisible:NSMakeRange([self.textView.text length], 0)];
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
    self.textView.text = @"中國象棋\n";
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    
    [self demoEngineUsage];
}

- (void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear:animated];
}

- (void)viewDidDisappear:(BOOL)animated
{
    [super viewDidDisappear:animated];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
    engineQuit();
}

@end
