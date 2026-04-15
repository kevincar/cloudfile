#import <Foundation/Foundation.h>
#import <FileProvider/FileProvider.h>

int materialize(const char *path) {
    @autoreleasepool {
        NSString *filePath = [NSString stringWithUTF8String:path];
        NSURL *fileURL = [NSURL fileURLWithPath:filePath];
        NSFileManager *fileManager = [NSFileManager defaultManager];

        NSError *error = nil;
        if (![fileManager startDownloadingUbiquitousItemAtURL:fileURL error:&error]) {
            NSLog(@"Error materializing file: %@", error);
            return 1;
        }

        NSLog(@"Requested materialization of file: %@", filePath);
        return 0;
    }
}

int evict(const char *path) {
    @autoreleasepool {
        NSString *filePath = [NSString stringWithUTF8String:path];
        NSURL *fileURL = [NSURL fileURLWithPath:filePath];
        NSFileManager *fileManager = [NSFileManager defaultManager];

        NSError *error = nil;
        if (![fileManager evictUbiquitousItemAtURL:fileURL error:&error]) {
            NSLog(@"Error evicting file: %@", error);
            return 1;
        }

        NSLog(@"Evicted file: %@", filePath);
        return 0;
    }
}
