const os = require('os');
const path = require('path');
const fs = require('fs');
// const fse = require('fs-extra');

const {
    shell,
    ipcRenderer
} = require('electron');
const url = require('url');

const _openInExternal = function(link) {
    let protocol = url.parse(link).protocol;
    if (protocol === 'http:' || protocol === 'https:') {
        shell.openExternal(link);
        return true;
    } else {
        return false;
    }
}

console.log("jQuery: " + $.fn.jquery);

let gConfig = {};
let iOSEle = null;
let androidEle = null;

$(document).ready(() => {
    ipcRenderer.send('config');
    ipcRenderer.on('config', function (event, config) {
        console.log('get config success');
        console.log(config);
        gConfig = config;
        updateUI(config);
    });

    $("#adcolony-active").change(function() {
        gConfig.active = $("#adcolony-active").is(':checked');
        ipcRenderer.send('saveConfig', gConfig);
    });

    $("#btn-open").click(function() {
        const path = $("#project-path").text();
        if (path) {
            shell.showItemInFolder(path);
            shell.beep();
        }
    });
    $("#btn-update").click(function() {
        const cfg = grapConfigure();
        console.log(cfg);
        ipcRenderer.send('saveConfig', cfg);
    });

    androidEle = $('.platform-config-area');
    if (1 == androidEle.length) {
        androidEle.find('button').click(function() {
            appendOrUpdateConfigAdsUI(androidEle.find('.adcolony-config-ads'));
        });

        iosEle = androidEle.clone();
        iosEle.find('.platform-name').text('iOS');
        iosEle.find('button').click(function() {
            appendOrUpdateConfigAdsUI(iosEle.find('.adcolony-config-ads'));
        });
        $('body').append(iosEle);
    } else {
        console.log('ERROR, class platform-config-area size is: ' + androidEle.length);
    }


    // const saveConfig = function() {
    //     const cfg = grapConfigure();
    //     ipcRenderer.send('saveConfig', cfg);
    // }
    // $(window).on('focusout', saveConfig);
    // $(window).on('unload', saveConfig);
})

$(document).on('click', 'a[href^="http"]', function(event) {
    if (_openInExternal(event.target.href)) {
        event.preventDefault();
    }
});

function grapPlatfromConfigure(eleUI) {
    const config = {};

    config.id = eleUI.find('.adcolony-config-id').val();
    config.debug = eleUI.find('.adcolony-config-debug').prop('checked');
    config.ads = {};
    eleUI.find(".adcolony-config-ads").children().each(function() {
        const child = $(this);
        const adCfg = {};
        adCfg.name = child.find('.adcolony-config-ad-name').val();
        adCfg.zone = child.find('.adcolony-config-ad-zone').val();
        if (adCfg.name && adCfg.zone) {
            config.ads[adCfg.name] = adCfg;
        }
    });

    return config;
}

function grapConfigure() {
    // const config = {};
    // const cb = $("#adcolony-active");
    // // config.active = cb.prop('checked');
    // //cb.prop('checked', true)
    // console.log(`checkbox status: ${cb.prop('checked')}`);
    // const pp = $("#project-path");
    // console.log(`project path: ${pp.text()}`);
    // const iv = $("#import-version");
    // console.log(`import version: ${iv.text()}`);

    const adcolonyAndroid = grapPlatfromConfigure(androidEle);
    const adcolonyIOS = grapPlatfromConfigure(iosEle);

    return {
        adcolonyCfg: {
            android: {
                AdColony: adcolonyAndroid
            },
            ios: {
                AdColony: adcolonyIOS
            }
        }
    };
}

function appendOrUpdateConfigAdsUI(eleAdsUI, adConfig) {
    let ele = null;
    if (adConfig) {
        eleAdsUI.children().each(function() {
            const child = $(this);
            let zoneId = child.find('.adcolony-config-ad-zone').val();
            if (adConfig.zone == zoneId) {
                ele = child;
                return false;
            }
        });
    }
    let needAppend = false;
    if (null == ele) {
        ele = eleAdsUI.children().first().clone();
        ele.css('display', 'flex');
        ele.find('.adcolony-config-ad-del').click(function(evt) {
            $(evt.target).parent().remove();
        });
        needAppend = true;
    }

    if (ele && adConfig) {
        ele.find('.adcolony-config-ad-name').val(adConfig.name);
        ele.find('.adcolony-config-ad-zone').val(adConfig.zone);
    }

    if (needAppend) {
        if (eleAdsUI.children().length < 16) {
            eleAdsUI.append(ele);
        } else {
            alert("广告项最多只能添加 15 个");
        }
    }
}

function updateConfigUI(eleUI, config) {
    if (!config) {
        return;
    }
    eleUI.find('.adcolony-config-id').val(config.id);
    eleUI.find('.adcolony-config-debug').attr('checked', config.debug);
    for (let key in config.ads) {
        config.ads[key]['name'] = key;
        appendOrUpdateConfigAdsUI(eleUI.find('.adcolony-config-ads'), config.ads[key]);
    }
}

function updateUI(cfg) {
    const cb = $("#adcolony-active");
    cb.attr('checked', cfg.active);
    // const pp = $("#project-path");
    // pp.text(cfg.projectPath);
    const iv = $("#import-version");
    iv.text(cfg.adcolonyVer);

    if (cfg.adcolonyCfg) {
        if (cfg.adcolonyCfg.android) {
            updateConfigUI(androidEle, cfg.adcolonyCfg.android.AdColony);
        }
        if (cfg.adcolonyCfg.ios) {
            updateConfigUI(iosEle, cfg.adcolonyCfg.ios.AdColony);
        }
    }
}







