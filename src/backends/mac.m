#import <Foundation/Foundation.h>
#import <FileProvider/FileProvider.h>

int cloudfile_is_verbose(void);

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

        if (cloudfile_is_verbose()) {
            printf("Materializing %s\n", [[filePath lastPathComponent] UTF8String]);
        }
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

        if (cloudfile_is_verbose()) {
            printf("Evicting %s\n", [[filePath lastPathComponent] UTF8String]);
        }
        return 0;
    }
}

int get_cloudfile_status(const char *path, int *status_code) {
    @autoreleasepool {
        NSString *filePath = [NSString stringWithUTF8String:path];
        NSURL *fileURL = [NSURL fileURLWithPath:filePath];

        NSNumber *isUbiquitousItem = nil;
        NSError *error = nil;
        if (![fileURL getResourceValue:&isUbiquitousItem forKey:NSURLIsUbiquitousItemKey error:&error]) {
            NSLog(@"Error checking file status: %@", error);
            return 1;
        }

        if (![isUbiquitousItem boolValue]) {
            NSLog(@"File is not managed as a cloud item: %@", filePath);
            return 1;
        }

        NSString *downloadStatus = nil;
        if (![fileURL getResourceValue:&downloadStatus
                                forKey:NSURLUbiquitousItemDownloadingStatusKey
                                 error:&error]) {
            NSLog(@"Error checking file status: %@", error);
            return 1;
        }

        if ([downloadStatus isEqualToString:NSURLUbiquitousItemDownloadingStatusNotDownloaded]) {
            *status_code = 0;
            return 0;
        }

        if ([downloadStatus isEqualToString:NSURLUbiquitousItemDownloadingStatusDownloaded] ||
            [downloadStatus isEqualToString:NSURLUbiquitousItemDownloadingStatusCurrent]) {
            *status_code = 1;
            return 0;
        }

        NSLog(@"Unknown cloud status for file %@: %@", filePath, downloadStatus);
        return 1;
    }
}
