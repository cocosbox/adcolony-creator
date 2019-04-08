package cocos.plugin;

import android.app.Activity;
import android.util.Log;

import com.adcolony.sdk.AdColony;
import com.adcolony.sdk.AdColonyAdOptions;
import com.adcolony.sdk.AdColonyAppOptions;
import com.adcolony.sdk.AdColonyInterstitial;
import com.adcolony.sdk.AdColonyInterstitialListener;
import com.adcolony.sdk.AdColonyReward;
import com.adcolony.sdk.AdColonyRewardListener;
import com.adcolony.sdk.AdColonyUserMetadata;
import com.adcolony.sdk.AdColonyZone;

import org.json.JSONException;
import org.json.JSONObject;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Iterator;

public class AdColonyWrapper {

    private final static int ADCOLONY_ZONE_STATUS_NO_ZONE = 0;
    private final static int ADCOLONY_ZONE_STATUS_OFF = 1;
    private final static int ADCOLONY_ZONE_STATUS_LOADING = 2;
    private final static int ADCOLONY_ZONE_STATUS_ACTIVE = 3;
    private final static int ADCOLONY_ZONE_STATUS_UNKNOWN = 4;

    private class AdColonyAdData {
        public String zone;
        public String name;
        boolean v4vc;
        boolean pre_popup;
        boolean post_popup;
        public AdColonyInterstitial interstital; // weak ref
        private int status;

        public void setStatus(int s) {
            status = s;
            jniSetAdColonyAdDataStatusWrapper(zone, status);
        }
    }

    private static String TAG = "AdColonyWrapper";
    private static AdColonyWrapper gInstance;
    private Activity mActivity;
    protected Method mReflectRunOnGLThread;

    private String mAdcolonyId;
    private boolean mDebug;
    private boolean mGDPR;
    private ArrayList<AdColonyAdData> mAdDatas;

    private AdColonyInterstitialListener mInterstitialListener;
    private AdColonyRewardListener mRewardListener;

    public static AdColonyWrapper getInstance() {
        if (null == gInstance) {
            gInstance = new AdColonyWrapper();
        }
        return gInstance;
    }

    public static void create(Activity act) {
        AdColonyWrapper.getInstance().setActivity(act);
    }

    /*
     * Interface For JNI
     */
    public static boolean nativeInit(String json) {
        return AdColonyWrapper.getInstance().init(json);
    }

    public static void nativeSetUserID(String uid) {
        AdColonyWrapper.getInstance().setUserID(uid);
    }

    public static void nativeShowAd(String zone) {
        AdColonyWrapper.getInstance().showAd(zone);
    }

    public static String nativeGetAdvertisingID() {
        return AdColonyWrapper.getInstance().getAdvertisingID();
    }

    public static void nativeSetUserMetadata(String key, String val) {
        AdColonyWrapper.getInstance().setUserMetadata(key, val);
    }

    public static void nativeUserInterestedIn(String topic) {
        AdColonyWrapper.getInstance().userInterestedIn(topic);
    }

    public static void nativeNotifyIAPComplete(String transactionID, String productID,
                                         int quantity, float price, String currentCode) {
        AdColonyWrapper.getInstance().notifyIAPComplete(transactionID,
                productID, quantity, price, currentCode);
    }

    public static void nativeRequestIntersitalAd(String adName) {
        AdColonyWrapper.getInstance().requestInterstitialAd(adName);
    }

    /*
     * JNI Interface End
     */

    /*
     * Native Function
     */
    private void jniSetAdColonyAdDataStatusWrapper(final String zone, final int status) {
        runOnGLThread(new Runnable() {
            @Override
            public void run() {
                jniSetAdColonyAdDataStatus(zone, status);
            }
        });
    }
    private void jniOnAdColonyChangeWrapper(final String zone, final boolean available) {
        runOnGLThread(new Runnable() {
            @Override
            public void run() {
                jniOnAdColonyChange(zone, available);
            }
        });
    }
    private void jniOnAdColonyRewardWrapper(final String zone, final String currencyName, final int amount, final boolean success) {
        runOnGLThread(new Runnable() {
            @Override
            public void run() {
                jniOnAdColonyReward(zone, currencyName, amount, success);
            }
        });
    }
    private void jniOnAdColonyStartedWrapper(final String zone) {
        runOnGLThread(new Runnable() {
            @Override
            public void run() {
                jniOnAdColonyStarted(zone);
            }
        });
    }
    private void jniOnAdColonyFinishedWrapper(final String zone) {
        runOnGLThread(new Runnable() {
            @Override
            public void run() {
                jniOnAdColonyFinished(zone);
            }
        });
    }
    private void jniOnAdColonyIapOpportunityWrapper(final String zone, final String iapProductID, final int iapEngagementType) {
        runOnGLThread(new Runnable() {
            @Override
            public void run() {
                jniOnAdColonyIapOpportunity(zone, iapProductID, iapEngagementType);
            }
        });
    }

    private static native void jniSetAdColonyAdDataStatus(String zone, int status);
    private static native void jniOnAdColonyChange(String zone, boolean available);
    private static native void jniOnAdColonyReward(String zone, String currencyName, int amount, boolean success);
    private static native void jniOnAdColonyStarted(String zone);
    private static native void jniOnAdColonyFinished(String zone);
    private static native void jniOnAdColonyIapOpportunity(String zone, String iapProductID, int iapEngagementType);
    /*
      * Native Function End
     */


    private AdColonyWrapper() {
        mAdDatas = new ArrayList<AdColonyAdData>();
    }

    public void setActivity(Activity act) {
        mActivity = act;

        Class<?> cls = act.getClass();
        try {
            mReflectRunOnGLThread = cls.getMethod("runOnGLThread", Runnable.class);
        } catch (NoSuchMethodException e) {
            Log.e(TAG, "can't reflect method runOnGLThread on Activity param");
            mReflectRunOnGLThread = null;
        }
    }

    public boolean init(String json) {
        if (null == json || 0 == json.length()) {
            Log.w(TAG, "config json is empty");
            return false;
        }
        //read config from json
        try {
            JSONObject jsonObject = new JSONObject(json);

            mAdcolonyId = jsonObject.optString("id");
            if (null == mAdcolonyId || 0 == mAdcolonyId.length()) {
                Log.w(TAG, "adcolony id is null");
                return false;
            }
            mDebug = jsonObject.optBoolean("debug");
            mGDPR = jsonObject.optBoolean("gdpr");

            mAdDatas.clear();
            JSONObject adsJsonObj = jsonObject.getJSONObject("ads");
            Iterator<String> it = adsJsonObj.keys();
            while (it.hasNext()) {
                String name = it.next();
                JSONObject adJsonbj =  adsJsonObj.getJSONObject(name);
                AdColonyAdData adData = new AdColonyAdData();
                adData.name = name;
                adData.zone = adJsonbj.optString("zone");
                adData.v4vc = adJsonbj.optBoolean("v4vc");
                adData.pre_popup = adJsonbj.optBoolean("pre_popup");
                adData.post_popup = adJsonbj.optBoolean("post_popup");

                if (null == adData.zone) {
                    Log.w(TAG, "can't find zone in config ads");
                }
                mAdDatas.add(adData);
            }
        } catch (JSONException e) {
            //eat it
        }

        //create insterstitial listener
        mInterstitialListener = new AdColonyInterstitialListener() {
            @Override
            public void onRequestFilled( AdColonyInterstitial ad ) {
                AdColonyAdData adData = findAdColonyAdData(ad.getZoneID());
                if (null == adData) {
                    Log.w(TAG, "can't find AdColonyAdData");
                    return;
                }
                adData.setStatus(ADCOLONY_ZONE_STATUS_ACTIVE);
                adData.interstital = ad;
                jniOnAdColonyChangeWrapper(adData.zone, true);
            }

            @Override
            public void onRequestNotFilled( AdColonyZone adZone ) {
                AdColonyAdData adData = findAdColonyAdData(adZone.getZoneID());
                if (null == adData) {
                    Log.w(TAG, "can't find AdColonyAdData");
                    return;
                }
                adData.setStatus(ADCOLONY_ZONE_STATUS_NO_ZONE);
                adData.interstital = null;
                jniOnAdColonyChangeWrapper(adData.zone, false);
            }

            @Override
            public void onOpened( AdColonyInterstitial ad ) {
                AdColonyAdData adData = findAdColonyAdData(ad.getZoneID());
                if (null == adData) {
                    Log.w(TAG, "can't find AdColonyAdData");
                    return;
                }
                jniOnAdColonyStartedWrapper(adData.zone);
            }

            @Override
            public void onExpiring( AdColonyInterstitial ad ) {
                AdColonyAdData adData = findAdColonyAdData(ad.getZoneID());
                if (null == adData) {
                    Log.w(TAG, "can't find AdColonyAdData");
                    return;
                }
                adData.setStatus(ADCOLONY_ZONE_STATUS_OFF);
                adData.interstital = null;
                jniOnAdColonyChangeWrapper(adData.zone, false);

                // request new interstital
                // requestInterstitialAd(adData);
            }

            @Override
            public void onClosed(AdColonyInterstitial ad) {
                AdColonyAdData adData = findAdColonyAdData(ad.getZoneID());
                if (null == adData) {
                    Log.w(TAG, "can't find AdColonyAdData");
                    return;
                }
                adData.setStatus(ADCOLONY_ZONE_STATUS_OFF);
                adData.interstital = null;
                jniOnAdColonyFinishedWrapper(adData.zone);

                // request new interstital
                // requestInterstitialAd(adData);
            }

            @Override
            public void onIAPEvent(AdColonyInterstitial ad, String product_id, int engagement_type) {
                AdColonyAdData adData = findAdColonyAdData(ad.getZoneID());
                if (null == adData) {
                    Log.w(TAG, "can't find AdColonyAdData");
                    return;
                }
                jniOnAdColonyIapOpportunityWrapper(adData.zone, product_id, engagement_type);

                // request new interstital
                // requestInterstitialAd(adData);
            }
        };

        mRewardListener = new AdColonyRewardListener() {
            @Override
            public void onReward( AdColonyReward reward ) {
                AdColonyAdData adData = findAdColonyAdData(reward.getZoneID());
                if (null == adData) {
                    Log.w(TAG, "can't find AdColonyAdData");
                    return;
                }
                jniOnAdColonyRewardWrapper(adData.zone, reward.getRewardName(), reward.getRewardAmount(), reward.success());
            }
        };

        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AdColonyAppOptions options = new AdColonyAppOptions();
                options.setTestModeEnabled(mDebug);
                options.setGDPRRequired(mGDPR);
                if (mGDPR) {
                    options.setGDPRConsentString("1");
                }

                String[] zones = new String[mAdDatas.size()];
                for (int i = 0; i < mAdDatas.size(); i++) {
                    zones[i] = mAdDatas.get(i).zone;
                }

                AdColony.configure(mActivity, options, mAdcolonyId, zones);
                AdColony.setRewardListener(mRewardListener);
                requestInterstitialAds();
            }
        });

        return false;
    }

    private void setUserID(String uid) {
        AdColony.getAppOptions().setUserID(uid);
    }

    private void showAd(final String zone) {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AdColonyAdData adData = findAdColonyAdData(zone);
                if (null == adData) {
                    Log.w(TAG, "can't find AdColonyAdData");
                    return;
                }
                if (null == adData.interstital) {
                    Log.w(TAG, "can't show ad, ad is not load or load failed");
                    return;
                }
                adData.interstital.show();
            }
        });

    }

    private String getAdvertisingID() {
        Log.w(TAG, "getAdvertisingID is invalid on android");
        return "";
    }

    private void setUserMetadata(String key, String val) {
        AdColonyUserMetadata meta = AdColony.getAppOptions().getUserMetadata();
        if (null == meta) {
            meta = new AdColonyUserMetadata();
        }
        meta.setMetadata(key, val);
        AdColony.getAppOptions().setUserMetadata(meta);
    }

    private void userInterestedIn(String topic) {
        AdColony.getAppOptions().getUserMetadata().addUserInterest(topic);
    }

    private void notifyIAPComplete(String transactionID, String productID,
                                         int quantity, float price, String currentCode) {
        AdColony.notifyIAPComplete(productID, transactionID, currentCode, price);
    }

    private void requestInterstitialAds() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                for (AdColonyAdData adData : mAdDatas) {
                    requestInterstitialAd(adData);
                }
            }
        });
    }

    private void requestInterstitialAd(AdColonyAdData adData) {
        AdColonyAdOptions options = new AdColonyAdOptions()
                .enableConfirmationDialog(adData.pre_popup)
                .enableResultsDialog(adData.post_popup);
        adData.setStatus(ADCOLONY_ZONE_STATUS_LOADING);
        AdColony.requestInterstitial(adData.zone, mInterstitialListener, options);
    }

    private void requestInterstitialAd(final String adName) {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                for (AdColonyAdData adData : mAdDatas) {
                    if (adData.name.equalsIgnoreCase(adName)) {
                        requestInterstitialAd(adData);
                        return;
                    }
                }
            }
        });
    }

    private AdColonyAdData findAdColonyAdData(String zone) {
        for (AdColonyAdData adData : mAdDatas) {
            if (adData.zone.equalsIgnoreCase(zone)) {
                return adData;
            }
        }

        return null;
    }

    private void runOnGLThread(Runnable r) {
        if (null == mReflectRunOnGLThread) {
            return;
        }
        if (null == mActivity) {
            return;
        }

        try {
            mReflectRunOnGLThread.invoke(mActivity, r);
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        }
    }

}
