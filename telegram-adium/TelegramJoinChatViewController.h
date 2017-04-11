/**
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 * Copyright Matthias Jentsch 2014-2015
 */

#import <Adium/DCJoinChatViewController.h>

@class AIAccount, AICompletingTextField;

@interface TelegramJoinChatViewController : DCJoinChatViewController <NSTokenFieldDelegate> {
  IBOutlet NSPopUpButton *popupButton_existingChat;
  IBOutlet NSTextField *textField_joinByLink;
  IBOutlet NSTextField *textField_createChatName;
  IBOutlet NSTokenField *tokenField_createChatUsers;
}

@end