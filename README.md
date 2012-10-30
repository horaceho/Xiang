Xiang
-----
Xiang is a port of the open source Chinese Chess engine "[ElephantEye](http://sourceforge.net/projects/xqwizard/)" to iOS. More information about ElephantEye can be found on [xqbase.com](http://www.xqbase.com/).

Use
---

To initialize the engine with an open book:

      NSString * bookPath = [[NSBundle mainBundle] pathForResource:@"OPENBOOK" ofType:@"DAT"];
      engineInit([bookPath UTF8String]);

To setup a board position:

      NSString *position = @"3akab2/1C6c/N3b4/9/1N7/9/9/C8/n4p3/rc2K1p2 w - - 0 1";
      engineSetFEN([position UTF8String]);

To start the engine:

      int timeLimitInSeconds = 30;
      unsigned long bestMove = engineThink(timeLimitInSeconds);

To shutdown the engine:

      engineQuit();

Please refer to `-demoEngineUsage` in `XiangViewController.m` for details.

License
-------
Xiang is released under the BSD license. ElephantEye engine is under the GNU Lesser General Public License. For more information about ElephantEye engine license, please refer to README.TXT and LGPL.TXT inside the ELEEYE folder. 

