//
//  JournalDocument.h
//  MoneyWise
//
//  Created by John Wiegley on Sun Jul 03 2005.
//  Copyright (c) 2005 New Artisans LLC. All rights reserved.
//

#include <ledger.h>

#import <Cocoa/Cocoa.h>

@interface JournalDocument : NSDocument
{
  ledger::journal_t * _journal;
}

- (ledger::journal_t *)journal;
@end
