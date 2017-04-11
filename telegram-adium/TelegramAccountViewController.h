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

#import <Adium/AIAccountViewController.h>
#import <AdiumLibpurple/PurpleAccountViewController.h>

@interface TelegramAccountViewController : PurpleAccountViewController {
  IBOutlet NSButton *checkbox_historySyncAll;
  IBOutlet NSButton *checkbox_displayReadNotifications;
  IBOutlet NSButton *checkbox_sendReadNotifications;
  
  IBOutlet NSTextField *textField_maxMsgSplitCount;
  IBOutlet NSTextField *textField_inactiveDaysOffline;
  IBOutlet NSTextField *textField_historyRetrieveDays;
  IBOutlet NSSecureTextField *textField_passwordTwoFactor;
  
  IBOutlet NSMatrix	*radio_Encryption;
}

@end
