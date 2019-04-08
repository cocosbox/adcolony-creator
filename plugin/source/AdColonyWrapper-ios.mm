
#import <AdColony/AdColony.h>
#import "AdColonyWrapper.h"

static std::map<std::string, AdColonyInterstitial *> g_zoneId2InterstitalMap;


#pragma mark AdColonyWrapper

namespace cocos {
    namespace plugin {
        
#pragma mark util

        UIViewController* getCurrentRootViewController() {
            UIViewController *result = nil;
            
            UIWindow *topWindow = [[UIApplication sharedApplication] keyWindow];
            if (topWindow.windowLevel != UIWindowLevelNormal) {
                NSArray *windows = [[UIApplication sharedApplication] windows];
                for(topWindow in windows) {
                    if (topWindow.windowLevel == UIWindowLevelNormal)
                        break;
                }
            }
            UIView *rootView = [[topWindow subviews] objectAtIndex:0];
            id nextResponder = [rootView nextResponder];
            if ([nextResponder isKindOfClass:[UIViewController class]]) {
                result = nextResponder;
            } else if ([topWindow respondsToSelector:@selector(rootViewController)] && topWindow.rootViewController != nil) {
                result = topWindow.rootViewController;
            } else {
                result = [[[[UIApplication sharedApplication] delegate] window] rootViewController];
                if (result == nil) {
                    NSLog(@"Could not find a root view controller.");
                }
            }
            
            return result;
        }

        bool AdColonyWrapper::nativeInit() {
            NSLog(@"AdColony Version:%@", [AdColony getSDKVersion]);
            
            if (adcolonyId.empty()) {
                NSLog(@"AdColony Config id is null");
                return false;
            }
            if (0 == ads.size()) {
                NSLog(@"AdColony Config Ads size is 0");
                return false;
            }
            
            NSMutableArray* zIDs = [NSMutableArray array];
            for (auto it = ads.begin(); it != ads.end(); ++it) {
                [zIDs addObject:[NSString stringWithUTF8String: it->second.zone.c_str()]];
            }
            
            AdColonyAppOptions *options = [[AdColonyAppOptions alloc] init];
            options.disableLogging = !debug;
            if (!uid.empty()) {
                options.userID = [NSString stringWithUTF8String:uid.c_str()];
            }
            if (gdpr) {
                options.gdprRequired = TRUE;
                options.gdprConsentString = @"1";
            } else {
                options.gdprRequired = NO;
                options.gdprConsentString = @"0";
            }

            [AdColony configureWithAppID:[NSString stringWithUTF8String:adcolonyId.c_str()]
                                 zoneIDs:zIDs
                                 options:options
                              completion:^(NSArray<AdColonyZone *> * zones) {
                                  for (AdColonyZone *zone in zones) {
                                      zone.reward = ^(BOOL success, NSString *name, int amount) {
                                          NSString *zoneId = [NSString stringWithString:[zone identifier]];
                                          nativeOnAdColonyReward(zoneId.UTF8String,
                                                                 name.UTF8String,
                                                                 amount,
                                                                 success);
                                      };
                                  }
                                  nativeRequestIntersitalAds();
                              }
             ];

            return true;
        }
        
        void AdColonyWrapper::nativeSetUserID(const std::string& u) {
            if (!initialized) {
                return;
            }
            if ([AdColony getAppOptions]) {
                [[AdColony getAppOptions] setUserID:[NSString stringWithUTF8String:u.c_str()]];
            }
        }

        void AdColonyWrapper::nativeRequestIntersitalAds() {
            for (auto &it : ads) {
                nativeRequestIntersitalAd(it.second);
            }
        }

        void AdColonyWrapper::nativeRequestIntersitalAd(AdColonyAdData &adData) {
            AdColonyAdOptions* options = [[AdColonyAdOptions alloc] init];
            options.showPrePopup = adData.pre_popup;
            options.showPostPopup = adData.post_popup;
            adData.status = ADCOLONY_ZONE_STATUS_LOADING;

            [AdColony requestInterstitialInZone:[NSString stringWithUTF8String:adData.zone.c_str()]
                                        options:options
                                        success:^(AdColonyInterstitial* interstitialAd) {

                                            adData.status = ADCOLONY_ZONE_STATUS_ACTIVE;
                                            g_zoneId2InterstitalMap[adData.zone] = interstitialAd;    // weak ref

                                            nativeOnAdColonyChange(adData.zone, true);
                                            
                                            interstitialAd.open = ^{
                                                nativeOnAdColonyStarted(adData.zone);
                                            };
                                            
                                            interstitialAd.close = ^{
                                                g_zoneId2InterstitalMap.erase(adData.zone);
                                                adData.status = ADCOLONY_ZONE_STATUS_OFF;

                                                nativeOnAdColonyFinished(adData.zone);
                                                
                                                // request new interstital
                                                // nativeRequestIntersitalAd(adData);
                                            };
                                            
                                            interstitialAd.expire = ^{
                                                g_zoneId2InterstitalMap.erase(adData.zone);
                                                adData.status = ADCOLONY_ZONE_STATUS_OFF;

                                                nativeOnAdColonyChange(adData.zone, false);

                                                // request new interstital
                                                // nativeRequestIntersitalAd(adData);
                                            };
                                            
                                            [interstitialAd setIapOpportunity:^(NSString * _Nonnull iapProductID, AdColonyIAPEngagement engagement) {
                                                nativeOnAdColonyIapOpportunity(adData.zone,
                                                                               iapProductID.UTF8String,
                                                                               engagement);
                                            }];
                                        }
                                        failure:^(AdColonyAdRequestError* error) {
                                            NSLog(@"AdColony Request Error:%@", error.description);
                                            adData.status = ADCOLONY_ZONE_STATUS_NO_ZONE;
                                            nativeOnAdColonyChange(adData.zone, false);
                                        }
             ];
        }

        void AdColonyWrapper::nativeShowAd(AdColonyAdData *adData) {
            if (nullptr == adData){
                return ;
            }

            auto it = g_zoneId2InterstitalMap.find(adData->zone);
            if (it != g_zoneId2InterstitalMap.end()) {
                [it->second showWithPresentingViewController:getCurrentRootViewController()];
            } else {
                NSLog(@"Can't find Ad for zone:%@", [NSString stringWithUTF8String:adData->zone.c_str()]);
            }
        }

        int AdColonyWrapper::nativeAdStatus(const std::string& zone) {
            AdColonyAdData adData;
            for (const auto &a : ads) {
                if (a.second.zone == zone) {
                    adData = a.second;
                    break;
                }
            }

            if (!adData.name.empty()) {
                return adData.status;
            } else {
                NSLog(@"invalid zone ID: %@", [NSString stringWithUTF8String: zone.c_str()] );
                return ADCOLONY_ZONE_STATUS_NO_ZONE;
            }
        }

        std::string AdColonyWrapper::getAdvertisingID() {
            return std::string([[AdColony getAdvertisingID] UTF8String]);
        }

        void AdColonyWrapper::setUserMetadata(const std::string &metadataType, const std::string &value) {
            [[AdColony getAppOptions] setOption:[NSString stringWithUTF8String:metadataType.c_str()]
                                withStringValue:[NSString stringWithUTF8String:value.c_str()]];
        }
        
        void AdColonyWrapper::userInterestedIn(const std::string &topic) {
            NSArray* interests = [NSArray arrayWithObject:[NSString stringWithUTF8String:topic.c_str()]];
            [[[AdColony getAppOptions] userMetadata] setUserInterests:interests];
        }
        
        void AdColonyWrapper::notifyIAPComplete(const std::string& transactionID,
                                                const std::string &productID,
                                                int quantity,
                                                float price,
                                                const std::string& currencyCode) {
            [AdColony iapCompleteWithTransactionID:[NSString stringWithUTF8String:transactionID.c_str()]
                                         productID:[NSString stringWithUTF8String:productID.c_str()]
                                             price:[NSNumber numberWithFloat:price]
                                      currencyCode:[NSString stringWithUTF8String:currencyCode.c_str()]];
        }

    }
}

