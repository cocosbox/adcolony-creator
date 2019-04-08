
#ifndef __AdColonyWrapper_h__
#define __AdColonyWrapper_h__

#include <string>
#include <map>

#define PLUGIN_LOG(...) cocos2d::log(__VA_ARGS__)

namespace cocos {
    namespace plugin {
        
        typedef enum {
            /**< AdColony has not been configured with that zone ID. */
            ADCOLONY_ZONE_STATUS_NO_ZONE = 0,
            /**< The zone has been turned off on the [Control Panel](http://clients.adcolony.com). */
            ADCOLONY_ZONE_STATUS_OFF,
            /**< The zone is preparing ads for display. */
            ADCOLONY_ZONE_STATUS_LOADING,
            /**< The zone has completed preparing ads for display. */
            ADCOLONY_ZONE_STATUS_ACTIVE,
            /**< AdColony has not yet received the zone's configuration from the server. */
            ADCOLONY_ZONE_STATUS_UNKNOWN
        } AdColonyAdStatus;
        
        struct AdColonyAdData {
            std::string name;
            std::string zone;
            bool v4vc;
            bool pre_popup;
            bool post_popup;
            int status;

//            static AdColonyData from_json(Json j) {
//                AdColonyData d;
//                d.zone = j["zone"].string_value("");
//                d.v4vc = j["v4vc"].bool_value(false);
//                d.pre_popup = j["pre_popup"].bool_value(false);
//                d.post_popup = j["post_popup"].bool_value(false);
//
//                return d;
//            }
        };
        
        struct AdColonyAdInfo {
            std::string name;
            bool shown;
            std::string zoneID;
            bool iapEnabled;
            std::string iapProductID;
            int iapQuantity;
            int iapEngagementType;
        };


        class AdColonyWrapperListener{
        public:
            virtual void onAdColonyChange(const AdColonyAdInfo& info,
                                          bool available) = 0;
            virtual void onAdColonyReward(const AdColonyAdInfo& info,
                                          const std::string& currencyName,
                                          int amount,
                                          bool success) = 0;
            virtual void onAdColonyStarted(const AdColonyAdInfo& info) = 0;
            virtual void onAdColonyFinished(const AdColonyAdInfo& info) = 0;
            virtual void onAdColonyIapOpportunity(const AdColonyAdInfo& info) = 0;
        };
        
        class AdColonyWrapper {
        public:
            AdColonyWrapper();
            ~AdColonyWrapper();

            static AdColonyWrapper *getInstance();

            void init();
            void setListener(AdColonyWrapperListener *listener);
            AdColonyWrapperListener *getListener();
            void removeListener();

            void show(const std::string &name);

            void setUserID(const std::string &uid);
            std::string getUserID();

            std::string getAdvertisingID();
            void setUserMetadata(const std::string &metadataType, const std::string &value);
            void userInterestedIn(const std::string &topic);
            void notifyIAPComplete(const std::string &transactionID,
                                   const std::string &productID,
                                   int quantity,
                                   float price,
                                   const std::string &currencyCode);
            void setGDPR(bool enabled);

            AdColonyAdStatus getAdStatus(const std::string &name);

            void requestIntersitalAd(const std::string &name);

//            int zoneStatusForZone(const std::string &zoneID);


        public:
            void setAdStatus(const std::string &name, AdColonyAdStatus status);
            AdColonyAdData findAdDataByZone(const std::string &zone);
            void nativeOnAdColonyChange(const std::string& zone, bool available);
            void nativeOnAdColonyReward(const std::string& zone,
                                        const std::string& currencyName,
                                        int amount,
                                        bool success);
            void nativeOnAdColonyStarted(const std::string& zone);
            void nativeOnAdColonyFinished(const std::string& zone);
            void nativeOnAdColonyIapOpportunity(const std::string& zone,
                                                const std::string iapProductID,
                                                int iapEngagementType);

        protected:
            bool readConfig(const std::string& filePath);

            AdColonyAdData findAdData(const std::string &name);

            bool nativeInit();
            void nativeSetUserID(const std::string& uid);
            void nativeRequestIntersitalAds();
            void nativeRequestIntersitalAd(AdColonyAdData &adData);
            void nativeShowAd(AdColonyAdData *adData);
            int nativeAdStatus(const std::string& zone);

        protected:
            bool initialized;
            AdColonyWrapperListener *listener;
            std::map<std::string, AdColonyAdData> ads;
            std::string adcolonyId;
            bool debug;
            bool gdpr;
            std::string uid;
            std::string cfgJson;

        private:
            static AdColonyWrapper *_instance;

        };
        
    }
}

#endif

