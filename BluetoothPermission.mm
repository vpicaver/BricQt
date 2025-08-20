#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

@interface BluetoothPermissionHelper : NSObject<CBCentralManagerDelegate>
@property (strong, nonatomic) CBCentralManager *manager;
@end

@implementation BluetoothPermissionHelper

- (instancetype)init {
    self = [super init];
    if (self) {
        dispatch_queue_t queue = dispatch_get_main_queue();
        self.manager = [[CBCentralManager alloc] initWithDelegate:self
                                                            queue:queue
                                                          options:@{ CBCentralManagerOptionShowPowerAlertKey: @NO }];
    }
    return self;
}

// This delegate method is called immediately; triggers permission prompt if needed
- (void)centralManagerDidUpdateState:(CBCentralManager *)central {
    // Nothing to do; just creating the manager forces the system to ask for permission
}

@end

extern "C" void requestBluetoothPermission() {
    static BluetoothPermissionHelper *helper;
    helper = [[BluetoothPermissionHelper alloc] init];
}
