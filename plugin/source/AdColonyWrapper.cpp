
#include "AdColonyWrapper.h"
#include "platform/CCFileUtils.h"

// #define JSON_IS_AMALGAMATION
// #include "json/json.h"
#include "cJSON.h"

namespace cocos {
    namespace plugin {
    
        AdColonyWrapper *AdColonyWrapper::_instance = nullptr;
        AdColonyWrapper* AdColonyWrapper::getInstance() {
            if (nullptr == _instance) {
                _instance = new cocos::plugin::AdColonyWrapper();
            }

            return _instance;
        }
        
        
        AdColonyWrapper::AdColonyWrapper()
        : initialized(false), listener(nullptr), gdpr(false), uid("") {
        }
        
        AdColonyWrapper::~AdColonyWrapper() {
        }
        
        void AdColonyWrapper::setUserID(const std::string &u) {
            uid = u;
            nativeSetUserID(uid);
        }

        std::string AdColonyWrapper::getUserID() {
            return uid;
        }

        void AdColonyWrapper::init() {
            if (initialized) {
                return;
            }
            initialized = true;
            
            readConfig("res/cocos_plugin_adcolony_config.json");
            nativeInit();
        }
        
        void AdColonyWrapper::show(const std::string &name) {
            if (!initialized) {
                PLUGIN_LOG("adcolony is not initialized.");
                return;
            }
            
            auto it = ads.find(name);
            if (it != ads.end()) {
                nativeShowAd(&it->second);
            } else {
                PLUGIN_LOG(":Failed to find ad with name: %s", name.c_str());
            }
        }
        
        void AdColonyWrapper::setListener(AdColonyWrapperListener *l) {
            listener = l;
        }
        
        AdColonyWrapperListener *AdColonyWrapper::getListener() {
            return listener;
        }
        
        void AdColonyWrapper::removeListener() {
            listener = nullptr;
        }
        
        AdColonyAdData AdColonyWrapper::findAdData(const std::string &name) {
            AdColonyAdData data = AdColonyAdData();
            
            auto it = ads.find(name);
            if (it != ads.end()) {
                data = it->second;
            } else {
                PLUGIN_LOG(":Failed to find ad with name: %s", name.c_str());
            }
            
            return data;
        }

        void AdColonyWrapper::setAdStatus(const std::string &zone, AdColonyAdStatus status) {
            for (auto &a : ads) {
                if (a.second.zone == zone) {
                    a.second.status = status;
                    break;
                }
            }
        }

        AdColonyAdData AdColonyWrapper::findAdDataByZone(const std::string &zone) {
            AdColonyAdData data;
            
            for (const auto &a : ads) {
                if (a.second.zone == zone) {
                    data = a.second;
                    return data;
                }
            }
            
            return data;
        }

        AdColonyAdStatus AdColonyWrapper::getAdStatus(const std::string &name) {
            auto it = ads.find(name);
            if (it != ads.end()) {
                AdColonyAdData& ad = it->second;
                return (AdColonyAdStatus) ad.status;
            }

            return ADCOLONY_ZONE_STATUS_NO_ZONE;
        }

        void AdColonyWrapper::setGDPR(bool enabled) {
            gdpr = enabled;
        }
        
        void AdColonyWrapper::requestIntersitalAd(const std::string &name) {
            auto it = ads.find(name);
            if (it != ads.end()) {
                AdColonyAdData& ad = it->second;
                if (AdColonyAdStatus::ADCOLONY_ZONE_STATUS_ACTIVE == ad.status) {
                    PLUGIN_LOG("WARNING: %s is active", name.c_str());
                }
                nativeRequestIntersitalAd(ad);
            }
        }

        bool AdColonyWrapper::readConfig(const std::string& filePath) {
            std::string content = cocos2d::FileUtils::getInstance()->getStringFromFile(filePath);

            cJSON* root = cJSON_Parse(content.c_str());
            if (nullptr == root) {
                PLUGIN_LOG("ERROR: parser config file failed");
                return false;
            }

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
            std::string platform = "ios";
#else
            std::string platform = "android";
#endif
            cJSON* cur = cJSON_GetObjectItemCaseSensitive(root, platform.c_str());
            if (nullptr == cur) {
                PLUGIN_LOG("ERROR: can't find platform: %s", platform.c_str());
                cJSON_Delete(root);
                return false;
            }

            cur = cJSON_GetObjectItemCaseSensitive(cur, "AdColony");
            if (nullptr == cur) {
                PLUGIN_LOG("ERROR: can't find AdColony");
                cJSON_Delete(root);
                return false;
            }
            cJSON* config = cur;

            cJSON* item = cJSON_GetObjectItemCaseSensitive(cur, "id");
            adcolonyId = cJSON_GetStringValue(item);
            item = cJSON_GetObjectItemCaseSensitive(cur, "debug");
            debug = (bool)cJSON_IsTrue(item);
            item = cJSON_GetObjectItemCaseSensitive(cur, "ads");
            if (nullptr == item) {
                PLUGIN_LOG("ERROR: can't find ads");
                cJSON_Delete(root);
                return false;
            }
            item = item->child;
            ads.clear();
            while(nullptr != item) {
                AdColonyAdData data;
                char* val;
                data.name = nullptr == item->string ? "" : item->string;
                val = cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(item, "zone"));
                data.zone = nullptr == val ? "" : val;
                data.v4vc = (bool)cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(item, "v4vc"));
                data.pre_popup = (bool)cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(item, "pre_popup"));
                data.post_popup = (bool)cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(item, "post_popup"));
                data.status = ADCOLONY_ZONE_STATUS_UNKNOWN;

                item = item->next;
                ads[data.name] = data;
            }

            cJSON_AddBoolToObject(config, "gdpr", gdpr);

            char* jsonstr = cJSON_Print(config);
            if (nullptr != jsonstr) {
                cfgJson = jsonstr;
                free(jsonstr);
            }

            cJSON_Delete(root);


            /*
            Json::CharReaderBuilder b;
            Json::CharReader* reader(b.newCharReader());
            JSONCPP_STRING errs;
            Json::Value root;
            if (!reader->parse(content.c_str(), content.c_str() + content.length(), &root, &errs)) {
                PLUGIN_LOG("ERROR: parser config failed:%s", errs.c_str());
                return false;
            }

            if (root.empty()) {
                PLUGIN_LOG("ERROR: config file is null");
                return false;
            }

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
            std::string platform = "ios";
#else
            std::string platform = "android";
#endif
            Json::Value config = root.get(platform, Json::objectValue).get("AdColony", Json::objectValue);

            adcolonyId = config.get("id", "").asString();
            if (adcolonyId.empty()) {
                PLUGIN_LOG("ERROR: Failed to find app id");
                return false;
            }
            debug = config.get("debug", false).asBool();

            Json::Value adsConfig = config.get("ads", Json::objectValue);
            if (adsConfig.empty()) {
                PLUGIN_LOG("ERROR: Failed to find ads for AdColony");
                return false;
            }

            ads.clear();
            for (auto i = adsConfig.begin(); i != adsConfig.end(); ++i) {
                AdColonyAdData data;
                data.name = i.name();
                data.zone = i->get("zone", "").asString();
                data.v4vc = i->get("v4vc", false).asBool();
                data.pre_popup = i->get("pre_popup", false).asBool();
                data.post_popup = i->get("post_popup", false).asBool();
                data.status = ADCOLONY_ZONE_STATUS_UNKNOWN;

                ads[i.name()] = data;
            }

            config["gdpr"] = gdpr;

            cfgJson = config.asString();
            */

            return true;
        }

        void AdColonyWrapper::nativeOnAdColonyChange(const std::string& zone, bool available) {
            if (nullptr == listener) {
                return;
            }
            
            AdColonyAdData adData = findAdDataByZone(zone);
            AdColonyAdInfo adInfo;
            adInfo.name = adData.name;
            adInfo.zoneID = adData.zone;
            adInfo.shown = false;
            adInfo.iapEnabled = false;
            adInfo.iapProductID = "";
            adInfo.iapQuantity = 0;
            adInfo.iapEngagementType = 0;

            listener->onAdColonyChange(adInfo, available);
        }

        void AdColonyWrapper::nativeOnAdColonyReward(const std::string& zone,
                                    const std::string& currencyName,
                                    int amount,
                                    bool success) {
            if (nullptr == listener) {
                return;
            }

            AdColonyAdData adData = findAdDataByZone(zone);
            AdColonyAdInfo adInfo;
            adInfo.name = adData.name;
            adInfo.zoneID = adData.zone;
            adInfo.shown = false;
            adInfo.iapEnabled = false;
            adInfo.iapProductID = "";
            adInfo.iapQuantity = 0;
            adInfo.iapEngagementType = 0;
            
            listener->onAdColonyReward(adInfo, currencyName, amount, success);
        }

        void AdColonyWrapper::nativeOnAdColonyStarted(const std::string& zone) {
            if (nullptr == listener) {
                return;
            }

            AdColonyAdData adData = findAdDataByZone(zone);
            AdColonyAdInfo adInfo;
            adInfo.name = adData.name;
            adInfo.zoneID = adData.zone;
            adInfo.shown = false;
            adInfo.iapEnabled = false;
            adInfo.iapProductID = "";
            adInfo.iapQuantity = 0;
            adInfo.iapEngagementType = 0;
            
            listener->onAdColonyStarted(adInfo);
        }

        void AdColonyWrapper::nativeOnAdColonyFinished(const std::string& zone) {
            if (nullptr == listener) {
                return;
            }
            
            AdColonyAdData adData = findAdDataByZone(zone);
            AdColonyAdInfo adInfo;
            adInfo.name = adData.name;
            adInfo.zoneID = adData.zone;
            adInfo.shown = false;
            adInfo.iapEnabled = false;
            adInfo.iapProductID = "";
            adInfo.iapQuantity = 0;
            adInfo.iapEngagementType = 0;
            
            listener->onAdColonyFinished(adInfo);
        }
        
        void AdColonyWrapper::nativeOnAdColonyIapOpportunity(const std::string& zone,
                                            const std::string iapProductID,
                                            int iapEngagementType) {
            if (nullptr == listener) {
                return;
            }
            
            AdColonyAdData adData = findAdDataByZone(zone);
            AdColonyAdInfo adInfo;
            adInfo.name = adData.name;
            adInfo.zoneID = adData.zone;
            adInfo.shown = false;
            adInfo.iapEnabled = false;
            adInfo.iapProductID = iapProductID;
            adInfo.iapQuantity = 0;
            adInfo.iapEngagementType = iapEngagementType;
            
            listener->onAdColonyIapOpportunity(adInfo);
        }

    }
}

