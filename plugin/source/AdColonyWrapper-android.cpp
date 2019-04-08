
#include "AdColonyWrapper.h"
// #include "cocos/platform/CCPlatformMacros.h"
#include "platform/android/jni/JniHelper.h"

namespace cocos {
    namespace plugin {

        bool AdColonyWrapper::nativeInit() {
            return cocos2d::JniHelper::callStaticBooleanMethod("cocos.plugin.AdColonyWrapper", "nativeInit", cfgJson);
        }
        
        void AdColonyWrapper::nativeSetUserID(const std::string& u) {
            if (!initialized) {
                return;
            }
            cocos2d::JniHelper::callStaticVoidMethod("cocos.plugin.AdColonyWrapper", "nativeSetUserID", u);
        }

        void AdColonyWrapper::nativeShowAd(AdColonyAdData *adData) {
            if (nullptr == adData){
                return ;
            }

            cocos2d::JniHelper::callStaticVoidMethod("cocos.plugin.AdColonyWrapper", "nativeShowAd", adData->zone);
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
                return ADCOLONY_ZONE_STATUS_NO_ZONE;
            }
        }

        std::string AdColonyWrapper::getAdvertisingID() {
            return cocos2d::JniHelper::callStaticStringMethod("cocos.plugin.AdColonyWrapper", "nativeGetAdvertisingID");
        }

        void AdColonyWrapper::setUserMetadata(const std::string &metadataType, const std::string &value) {
            cocos2d::JniHelper::callStaticVoidMethod("cocos.plugin.AdColonyWrapper", "nativeSetUserMetadata", metadataType, value);
        }
        
        void AdColonyWrapper::userInterestedIn(const std::string &topic) {
            cocos2d::JniHelper::callStaticVoidMethod("cocos.plugin.AdColonyWrapper", "nativeUserInterestedIn", topic);
        }
        
        void AdColonyWrapper::notifyIAPComplete(const std::string& transactionID,
                                                const std::string &productID,
                                                int quantity,
                                                float price,
                                                const std::string& currencyCode) {
            cocos2d::JniHelper::callStaticVoidMethod("cocos.plugin.AdColonyWrapper",
                                                    "nativeNotifyIAPComplete",
                                                    transactionID,
                                                    productID,
                                                    quantity,
                                                    price,
                                                    currencyCode);
        }

        void AdColonyWrapper::nativeRequestIntersitalAd(AdColonyAdData &adData) {
            cocos2d::JniHelper::callStaticVoidMethod("cocos.plugin.AdColonyWrapper",
                                                    "nativeRequestIntersitalAd",
                                                    adData.name);
        }

    }
}


//////////////////////////////////////////////////////////////////////////
// Jni callback function
//////////////////////////////////////////////////////////////////////////

extern "C" {

JNIEXPORT void JNICALL Java_cocos_plugin_AdColonyWrapper_jniSetAdColonyAdDataStatus(
    JNIEnv* env, jobject thiz, jstring jzone, jint status) {
        std::string zone = cocos2d::JniHelper::jstring2string(jzone);
        cocos::plugin::AdColonyWrapper::getInstance()
                ->setAdStatus(zone, (cocos::plugin::AdColonyAdStatus)status);
}

JNIEXPORT void JNICALL Java_cocos_plugin_AdColonyWrapper_jniOnAdColonyChange(
    JNIEnv* env, jobject thiz, jstring jzone, jboolean available) {
        std::string zone = cocos2d::JniHelper::jstring2string(jzone);
        cocos::plugin::AdColonyWrapper::getInstance()->nativeOnAdColonyChange(zone, available);
}

JNIEXPORT void JNICALL Java_cocos_plugin_AdColonyWrapper_jniOnAdColonyReward(
    JNIEnv* env, jobject thiz, jstring jzone, jstring jcurrencyName, jint amount, jboolean success) {
        std::string zone = cocos2d::JniHelper::jstring2string(jzone);
        std::string currencyName = cocos2d::JniHelper::jstring2string(jcurrencyName);
        cocos::plugin::AdColonyWrapper::getInstance()->nativeOnAdColonyReward(zone, currencyName, amount, success);
}

JNIEXPORT void JNICALL Java_cocos_plugin_AdColonyWrapper_jniOnAdColonyStarted(
    JNIEnv* env, jobject thiz, jstring jzone) {
        std::string zone = cocos2d::JniHelper::jstring2string(jzone);
        cocos::plugin::AdColonyWrapper::getInstance()->nativeOnAdColonyStarted(zone);
}

JNIEXPORT void JNICALL Java_cocos_plugin_AdColonyWrapper_jniOnAdColonyFinished(
    JNIEnv* env, jobject thiz, jstring jzone) {
        std::string zone = cocos2d::JniHelper::jstring2string(jzone);
        cocos::plugin::AdColonyWrapper::getInstance()->nativeOnAdColonyFinished(zone);
}

JNIEXPORT void JNICALL Java_cocos_plugin_AdColonyWrapper_jniOnAdColonyIapOpportunity(
    JNIEnv* env, jobject thiz, jstring jzone, jstring jiapProductID, jint iapEngagementType) {
        std::string zone = cocos2d::JniHelper::jstring2string(jzone);
        std::string iapProductID = cocos2d::JniHelper::jstring2string(jiapProductID);
        cocos::plugin::AdColonyWrapper::getInstance()->nativeOnAdColonyIapOpportunity(
            zone, iapProductID, iapEngagementType);
}

}

