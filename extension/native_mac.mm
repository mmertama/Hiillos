#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#include <iostream>
#include <set>
#include "../app/window_provider.h"
#include "native.h"

using namespace Native;

// Gempyre provides filters as {"FILTER_NAME":["*.filter1", "*.filter2"]}, wheere cocoa uses just "filter1", "filter2"
static auto getFilters(const FilterType& ft) {
    std::vector<std::string> vec;
    for(const auto& [k,v] : ft) {
        for(const auto& f : v) {
            const auto p = f.find_first_of('.');
            vec.push_back(f.substr(p + 1));
        }
    }
    return vec;
}

std::vector<std::string> Native::openDialog(
        const std::string& aTitle,
        const std::string& aDefaultPathAndFile,
        bool dirOnly,
        bool allowMultiple,
        const FilterType& filters) {

    std::vector<std::string> fileList;
    // Create a File Open Dialog class.
    NSOpenPanel* openDlg = [NSOpenPanel openPanel];

    [openDlg setCanChooseFiles: !dirOnly];
    [openDlg setCanChooseDirectories: dirOnly];
    [openDlg setAllowsMultipleSelection: allowMultiple];

    [openDlg setTitle: [NSString stringWithUTF8String:aTitle.c_str()]];

    [openDlg setLevel:CGShieldingWindowLevel()];

    // Set array of file types

    NSMutableArray * fileTypesArray = [NSMutableArray array];
    for (const auto& f : getFilters(filters)) {
        NSString * filt =[NSString stringWithUTF8String:f.c_str()];
        [fileTypesArray addObject:filt];
   }

   [openDlg setAllowedFileTypes:fileTypesArray];
   [openDlg setDirectoryURL:[NSURL URLWithString:[NSString stringWithUTF8String:aDefaultPathAndFile.c_str() ] ] ];

   // Display the dialog box. If the OK pressed,
   // process the files.
   if ( [openDlg runModal] == NSModalResponseOK ) {
      // Gets list of all files selected
      NSArray *files = [openDlg URLs];
      // Loop through the files and process them.
      for(unsigned i = 0; i < [files count]; i++ ) {
         // Do something with the filename.
        fileList.push_back(std::string([[[files objectAtIndex:i] path] UTF8String]));
        }
   }
   return fileList;
}

std::string Native::saveDialog (
     const std::string& aTitle ,
     const std::string& aDefaultPathAndFile,
     const FilterType& filters) {

   // Create a File Open Dialog class.
   NSSavePanel* openDlg = [NSSavePanel savePanel];
   [openDlg setTitle: [NSString stringWithUTF8String:aTitle.c_str()]];

   std::string fname;
   std::string dir = aDefaultPathAndFile;
   const auto pos = aDefaultPathAndFile.find_last_of('/');
   if(pos > 0 && pos != std::string::npos) {
       dir = aDefaultPathAndFile.substr(0, pos);
       fname = aDefaultPathAndFile.substr(pos + 1);
   }

   NSString* defaultPath = [NSString stringWithUTF8String:dir.c_str()];
   [openDlg setDirectoryURL:[NSURL fileURLWithPath:defaultPath]];

   NSString* defaultFile = [NSString stringWithUTF8String:fname.c_str()];
   [openDlg setNameFieldStringValue:defaultFile];

   [openDlg setCanCreateDirectories: TRUE];

   [openDlg setLevel:CGShieldingWindowLevel()];

   // Set array of file types

   NSMutableArray* fileTypesArray = [NSMutableArray array];
   for (const auto& f : getFilters(filters))
   {
      NSString* filt =[NSString stringWithUTF8String:f.c_str()];
      [fileTypesArray addObject:filt];
   }

   [openDlg setAllowedFileTypes:fileTypesArray];

   // Display the dialog box. If the OK pressed,
   // process the files.
   if ([openDlg runModal] == NSModalResponseOK) {
       NSString* url = [openDlg URL].resourceSpecifier;
       return std::string([url UTF8String]);
   }
   return "";
}

bool Native::setAppIcon(WindowProvider* prov, const std::vector<uint8_t>& data, const std::string& type) {
    (void) type; // TODO
    (void) prov;
    if(!data.empty()) {
        NSData* bytes = [NSData dataWithBytes:data.data() length:data.size()];
        NSImage* icon = [[NSImage alloc] initWithData: bytes];
        [[NSApplication sharedApplication] setApplicationIconImage: icon];
    } else {
        [[NSApplication sharedApplication] setApplicationIconImage: Nil];
    }
    return true;
}

void Native::debugPrint(int level, const char* line) {
    std::cerr << "WS(" << level << "):" << line << std::endl;
}

bool Native::setSize(WindowProvider* prov, int width, int height) {
    std::set<NSWindow*> native_windows;
    for(const auto w : prov->windows()) {
        NSView* view = (__bridge NSView*) w;
        NSWindow* win = [view window];
        native_windows.emplace(win);
    }
    /*
    NSWindow *topwin = [[[NSApplication sharedApplication] windows] objectAtIndex:0];
    native_windows.emplace(topwin);*/

    for(auto wh : native_windows) {
        const auto wp = (NSWindow*) wh;
        NSRect frame = [wp frame];
        frame.origin.y -= frame.size.height; // remove the old height
        frame.origin.y += height; // add the new height
        frame.size = {(CGFloat) width, (CGFloat) height};
        [wp setFrame:frame display:YES animate:YES];
    }
    return true;
}

bool Native::setTitle(WindowProvider* prov, const std::string& name) {
    NSMenu *menu = [[[NSApp mainMenu] itemAtIndex:0] submenu];
    NSString *title = [NSString stringWithCString:name.c_str()
            encoding:[NSString defaultCStringEncoding]];

    for(const auto w : prov->windows()) {
        NSView* view = (__bridge NSView*) w;
        NSWindow* win = [view window];
        win.title = title;
    }

    // Append some invisible character to title :)
    title = [title stringByAppendingString:@"\x1b"];

    [menu setTitle:title];
    return true;
}
