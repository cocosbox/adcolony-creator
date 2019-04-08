
#include "AdColonyJSHelper.h"
#include "AdColonyWrapper.h"
#include "scripting/js-bindings/manual/jsb_conversions.hpp"
#include "cocos2d.h"
#include "CCScheduler.h"
#include "platform/CCApplication.h"

#define MAKE_V8_HAPPY \
se::ScriptEngine::getInstance()->clearException(); \
se::AutoHandleScope hs;

se::Object* __jsb_cocos_plugin_AdColony_proto = nullptr;
se::Class* __jsb_cocos_plugin_AdColony_class = nullptr;


static se::Value adinfo_to_jsobj(const cocos::plugin::AdColonyAdInfo& info) {
    se::Value ret;
    cocos2d::ValueMap map;
    map["name"] = cocos2d::Value(info.name);
    map["zoneID"] = cocos2d::Value(info.zoneID);
    map["shown"] = cocos2d::Value(info.shown);
    map["iapEnabled"] = cocos2d::Value(info.iapEnabled);
    map["iapProductID"] = cocos2d::Value(info.iapProductID);
    map["iapQuantity"] = cocos2d::Value(info.iapQuantity);
    map["iapEngagementType"] = cocos2d::Value(info.iapEngagementType);
    ccvaluemap_to_seval(map, &ret);
    return ret;
}

class AdColonyListenerJS : public cocos::plugin::AdColonyWrapperListener {
public:
    AdColonyListenerJS() {
    }
    
    void setJSDelegate(const se::Value& jsDelegate) {
        _JSDelegate = jsDelegate;
    }
    
    const se::Value& getJSDelegate() {
        return _JSDelegate;
    }
    
    void invokeJSFun(const std::string& funName, const se::ValueArray& params = se::EmptyValueArray) {
        cocos2d::Application::getInstance()->getScheduler()->performFunctionInCocosThread([funName, params, this](){
            se::ScriptEngine::getInstance()->clearException();
            se::AutoHandleScope hs;
            if (!_JSDelegate.isObject())
                return;
            
            se::Value func;
            _JSDelegate.toObject()->getProperty(funName.c_str(), &func);
            
            if (func.isObject() && func.toObject()->isFunction()) {
                bool ok = func.toObject()->call(params, _JSDelegate.toObject());
                if (!ok) {
                    se::ScriptEngine::getInstance()->clearException();
                }
            }
        });
    }
    
    
    void onAdColonyChange(const cocos::plugin::AdColonyAdInfo& info, bool available) {
        MAKE_V8_HAPPY
        
        se::ValueArray args;
        args.reserve(2);
        args.push_back(adinfo_to_jsobj(info));
        args.push_back(se::Value(available));
        invokeJSFun(__FUNCTION__, args);
    }
    
    void onAdColonyReward(const cocos::plugin::AdColonyAdInfo& info,
                          const std::string& currencyName,
                          int amount,
                          bool success) {
        MAKE_V8_HAPPY
        
        se::ValueArray args;
        args.reserve(4);
        args.push_back(adinfo_to_jsobj(info));
        args.push_back(se::Value(currencyName));
        args.push_back(se::Value(amount));
        args.push_back(se::Value(success));
        invokeJSFun(__FUNCTION__, args);
    }
    
    void onAdColonyStarted(const cocos::plugin::AdColonyAdInfo& info) {
        MAKE_V8_HAPPY
        
        se::ValueArray args;
        args.reserve(1);
        args.push_back(adinfo_to_jsobj(info));
        invokeJSFun(__FUNCTION__, args);
    }
    
    void onAdColonyFinished(const cocos::plugin::AdColonyAdInfo& info) {
        MAKE_V8_HAPPY
        
        se::ValueArray args;
        args.reserve(1);
        args.push_back(adinfo_to_jsobj(info));
        invokeJSFun(__FUNCTION__, args);
    }
    
    void onAdColonyIapOpportunity(const cocos::plugin::AdColonyAdInfo& info) {
        MAKE_V8_HAPPY
        
        se::ValueArray args;
        args.reserve(1);
        args.push_back(adinfo_to_jsobj(info));
        invokeJSFun(__FUNCTION__, args);
    }
    
protected:
    se::Value _JSDelegate;
};




static bool js_CocosPluginAdColonyJS_AdColony_setListener(se::State& s) {
    const auto& args = s.args();
    int argc = (int)args.size();
    if (argc == 1) {
        auto wrapper = cocos::plugin::AdColonyWrapper::getInstance();
        AdColonyListenerJS* nativeDelegate = dynamic_cast<AdColonyListenerJS*>(wrapper->getListener());
        if (nullptr == nativeDelegate) {
            nativeDelegate = new (std::nothrow) AdColonyListenerJS();
            wrapper->setListener(nativeDelegate);
        }
        nativeDelegate->setJSDelegate(args[0]);
        
        return true;
    }
    
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 1);
    return false;
}
SE_BIND_FUNC(js_CocosPluginAdColonyJS_AdColony_setListener)

static bool js_CocosPluginAdColonyJS_AdColony_init(se::State& s) {
    const auto& args = s.args();
    int argc = (int)args.size();

    auto wrapper = cocos::plugin::AdColonyWrapper::getInstance();
    if (argc == 0) {
        wrapper->init();
        return true;
    }

    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 0);
    return false;
}
SE_BIND_FUNC(js_CocosPluginAdColonyJS_AdColony_init)

static bool js_CocosPluginAdColonyJS_AdColony_removeListener(se::State& s) {
    const auto& args = s.args();
    int argc = (int)args.size();
    
    auto wrapper = cocos::plugin::AdColonyWrapper::getInstance();
    if (argc == 0) {
        wrapper->removeListener();
        return true;
    }
    
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 0);
    return false;
}
SE_BIND_FUNC(js_CocosPluginAdColonyJS_AdColony_removeListener)

static bool js_CocosPluginAdColonyJS_AdColony_show(se::State& s) {
    const auto& args = s.args();
    size_t argc = args.size();
    CC_UNUSED bool ok = true;
    
    auto wrapper = cocos::plugin::AdColonyWrapper::getInstance();
    if (argc == 1) {
        std::string name;
        ok &= seval_to_std_string(args[0], &name);
        SE_PRECONDITION2(ok, false, "js_CocosPluginAdColonyJS_AdColony_show : Error processing arguments");
        wrapper->show(name);
        return true;
    }

    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", (int)argc, 1);
    return false;
}
SE_BIND_FUNC(js_CocosPluginAdColonyJS_AdColony_show)

static bool js_CocosPluginAdColonyJS_AdColony_setUserID(se::State& s) {
    const auto& args = s.args();
    int argc = (int)args.size();
    CC_UNUSED bool ok = true;
    
    auto wrapper = cocos::plugin::AdColonyWrapper::getInstance();
    if (argc == 1) {
        std::string uid;
        ok &= seval_to_std_string(args[0], &uid);
        SE_PRECONDITION2(ok, false, "js_CocosPluginAdColonyJS_AdColony_setUserID : Error processing arguments");
        wrapper->setUserID(uid);
        return true;
    }
    
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 1);
    return false;
}
SE_BIND_FUNC(js_CocosPluginAdColonyJS_AdColony_setUserID)

static bool js_CocosPluginAdColonyJS_AdColony_getUserID(se::State& s) {
    const auto& args = s.args();
    int argc = (int)args.size();
    CC_UNUSED bool ok = true;
    
    auto wrapper = cocos::plugin::AdColonyWrapper::getInstance();
    if (argc == 0) {
        std::string uid = wrapper->getUserID();
        ok &= std_string_to_seval(uid, &s.rval());
        SE_PRECONDITION2(ok, false, "js_CocosPluginAdColonyJS_AdColony_getUserID : Error processing arguments");
        return true;
    }
    
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 0);
    return false;
}
SE_BIND_FUNC(js_CocosPluginAdColonyJS_AdColony_getUserID)

static bool js_CocosPluginAdColonyJS_AdColony_getAdvertisingID(se::State& s) {
    const auto& args = s.args();
    int argc = (int)args.size();
    bool ok = true;

    auto wrapper = cocos::plugin::AdColonyWrapper::getInstance();
    if (argc == 0) {
        std::string aid = wrapper->getAdvertisingID();
        ok &= std_string_to_seval(aid, &s.rval());
        SE_PRECONDITION2(ok, false, "js_CocosPluginAdColonyJS_AdColony_getAdvertisingID : Error processing arguments");
        return true;
    }
    
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 0);
    return false;
}
SE_BIND_FUNC(js_CocosPluginAdColonyJS_AdColony_getAdvertisingID)

static bool js_CocosPluginAdColonyJS_AdColony_setUserMetadata(se::State& s) {
    const auto& args = s.args();
    int argc = (int)args.size();
    bool ok = true;
    
    auto wrapper = cocos::plugin::AdColonyWrapper::getInstance();
    if (argc == 2) {
        std::string key;
        std::string val;
        ok &= seval_to_std_string(args[0], &key);
        ok &= seval_to_std_string(args[1], &val);
        SE_PRECONDITION2(ok, false, "js_CocosPluginAdColonyJS_AdColony_setUserMetadata : Error processing arguments");
        wrapper->setUserMetadata(key, val);
        return true;
    }
    
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 2);
    return false;
}
SE_BIND_FUNC(js_CocosPluginAdColonyJS_AdColony_setUserMetadata)

static bool js_CocosPluginAdColonyJS_AdColony_userInterestedIn(se::State& s) {
    const auto& args = s.args();
    int argc = (int)args.size();
    bool ok = true;
    
    auto wrapper = cocos::plugin::AdColonyWrapper::getInstance();
    if (argc == 1) {
        std::string topic;
        ok &= seval_to_std_string(args[0], &topic);
        SE_PRECONDITION2(ok, false, "js_CocosPluginAdColonyJS_AdColony_userInterestedIn : Error processing arguments");
        wrapper->userInterestedIn(topic);
        return true;
    }
    
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 1);
    return false;
}
SE_BIND_FUNC(js_CocosPluginAdColonyJS_AdColony_userInterestedIn)

static bool js_CocosPluginAdColonyJS_AdColony_notifyIAPComplete(se::State& s) {
    const auto& args = s.args();
    int argc = (int)args.size();
    bool ok = true;
    
    auto wrapper = cocos::plugin::AdColonyWrapper::getInstance();
    if (argc == 5) {
        std::string transactionID;
        std::string productID;
        int quantity;
        float price;
        std::string currency;

        ok &= seval_to_std_string(args[0], &transactionID);
        ok &= seval_to_std_string(args[1], &productID);
        ok &= seval_to_int32(args[2], &quantity);
        ok &= seval_to_float(args[3], &price);
        ok &= seval_to_std_string(args[4], &currency);
        
        SE_PRECONDITION2(ok, false, "js_CocosPluginAdColonyJS_AdColony_notifyIAPComplete : Error processing arguments");
        wrapper->notifyIAPComplete(transactionID, productID, quantity, price, currency);
        return true;
    }
    
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 5);
    return false;
}
SE_BIND_FUNC(js_CocosPluginAdColonyJS_AdColony_notifyIAPComplete)

static bool js_CocosPluginAdColonyJS_AdColony_setGDPR(se::State& s) {
    const auto& args = s.args();
    int argc = (int)args.size();
    bool ok = true;

    auto wrapper = cocos::plugin::AdColonyWrapper::getInstance();
    if (argc == 1) {
        bool enable;

        ok &= seval_to_boolean(args[0], &enable);
        SE_PRECONDITION2(ok, false, "js_CocosPluginAdColonyJS_AdColony_setGDPR : Error processing arguments");

        wrapper->setGDPR(enable);
        return true;
    }
    
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 1);
    return false;
}
SE_BIND_FUNC(js_CocosPluginAdColonyJS_AdColony_setGDPR)

static bool js_CocosPluginAdColonyJS_AdColony_getAdStatus(se::State& s) {
    const auto& args = s.args();
    int argc = (int)args.size();
    bool ok = true;
    
    auto wrapper = cocos::plugin::AdColonyWrapper::getInstance();
    if (argc == 1) {
        std::string name;
        
        ok &= seval_to_std_string(args[0], &name);
        
        SE_PRECONDITION2(ok, false, "js_CocosPluginAdColonyJS_AdColony_getAdStatus : Error processing arguments");
        cocos::plugin::AdColonyAdStatus status = wrapper->getAdStatus(name);

        ok &= int32_to_seval(status, &s.rval());
        SE_PRECONDITION2(ok, false, "js_CocosPluginAdColonyJS_AdColony_getAdStatus : Error processing result");

        return true;
    }
    
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 1);
    return false;
}
SE_BIND_FUNC(js_CocosPluginAdColonyJS_AdColony_getAdStatus)

static bool js_CocosPluginAdColonyJS_AdColony_requestIntersitalAd(se::State& s) {
    const auto& args = s.args();
    int argc = (int)args.size();
    bool ok = true;
    
    auto wrapper = cocos::plugin::AdColonyWrapper::getInstance();
    if (argc == 1) {
        std::string name;
        
        ok &= seval_to_std_string(args[0], &name);
        
        SE_PRECONDITION2(ok, false, "js_CocosPluginAdColonyJS_AdColony_requestIntersitalAd : Error processing arguments");
        wrapper->requestIntersitalAd(name);
        
        return true;
    }
    
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 1);
    return false;
}
SE_BIND_FUNC(js_CocosPluginAdColonyJS_AdColony_requestIntersitalAd)

static bool js_CocosPluginAdColonyJS_AdColony_finalize(se::State& s) {
    CCLOGINFO("jsbindings: finalizing JS object %p (cocos::plugin::AdColonyWrapper)", s.nativeThisObject());
    auto iter = se::NonRefNativePtrCreatedByCtorMap::find(s.nativeThisObject());
    if (iter != se::NonRefNativePtrCreatedByCtorMap::end()) {
        se::NonRefNativePtrCreatedByCtorMap::erase(iter);
        cocos::plugin::AdColonyWrapper* cobj = (cocos::plugin::AdColonyWrapper*)s.nativeThisObject();
        delete cobj;
    }
    return true;
}
SE_BIND_FINALIZE_FUNC(js_CocosPluginAdColonyJS_AdColony_finalize)

static se::Value makeSureObjectExist(se::Object* obj, std::vector<std::string> &paths) {
    se::Object* curObj = obj;
    se::Value ret;
    
    std::vector<std::string> allKeys;

    for (auto n : paths) {
        if (!curObj->getProperty(n.c_str(), &ret)) {
            se::HandleObject jsobj(se::Object::createPlainObject());
            ret.setObject(jsobj, true);
            curObj->setProperty(n.c_str(), ret);
            jsobj->incRef();
        }
        allKeys.clear();
        curObj->getAllKeys(&allKeys);
        curObj = ret.toObject();
    }

    return ret;
}

bool register_all_CocosPluginAdColonyJS_AdColony_helper(se::Object* obj) {
    
    auto cls = se::Class::create("AdColony", obj, nullptr, nullptr);

    cls->defineStaticFunction("setListener", _SE(js_CocosPluginAdColonyJS_AdColony_setListener));
    cls->defineStaticFunction("init", _SE(js_CocosPluginAdColonyJS_AdColony_init));
    cls->defineStaticFunction("removeListener", _SE(js_CocosPluginAdColonyJS_AdColony_removeListener));
    cls->defineStaticFunction("show", _SE(js_CocosPluginAdColonyJS_AdColony_show));
    cls->defineStaticFunction("setUserID", _SE(js_CocosPluginAdColonyJS_AdColony_setUserID));
    cls->defineStaticFunction("getUserID", _SE(js_CocosPluginAdColonyJS_AdColony_getUserID));
    cls->defineStaticFunction("getAdvertisingID", _SE(js_CocosPluginAdColonyJS_AdColony_getAdvertisingID));
    cls->defineStaticFunction("setUserMetadata", _SE(js_CocosPluginAdColonyJS_AdColony_setUserMetadata));
    cls->defineStaticFunction("userInterestedIn", _SE(js_CocosPluginAdColonyJS_AdColony_userInterestedIn));
    cls->defineStaticFunction("notifyIAPComplete", _SE(js_CocosPluginAdColonyJS_AdColony_notifyIAPComplete));
    cls->defineStaticFunction("setGDPR", _SE(js_CocosPluginAdColonyJS_AdColony_setGDPR));
    cls->defineStaticFunction("getAdStatus", _SE(js_CocosPluginAdColonyJS_AdColony_getAdStatus));
    cls->defineStaticFunction("requestIntersitalAd", _SE(js_CocosPluginAdColonyJS_AdColony_requestIntersitalAd));

    cls->defineFinalizeFunction(_SE(js_CocosPluginAdColonyJS_AdColony_finalize));
    cls->install();
    JSBClassType::registerClass<cocos::plugin::AdColonyWrapper>(cls);
    
    __jsb_cocos_plugin_AdColony_proto = cls->getProto();
    __jsb_cocos_plugin_AdColony_class = cls;

    se::ScriptEngine::getInstance()->clearException();
    return true;
}

bool register_all_CocosPluginAdColonyJS_AdColony(se::Object* obj) {
    std::vector<std::string> paths;
    paths.push_back("cocos");
    paths.push_back("plugin");
    auto pluginObj = makeSureObjectExist(obj, paths);
    
    return register_all_CocosPluginAdColonyJS_AdColony_helper(pluginObj.toObject());
}


