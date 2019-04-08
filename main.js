'use strict';

const {
    dialog,
    ipcMain,
    BrowserWindow,
    globalShortcut
} = require('electron');

const os = require('os');
const FS = require("fire-fs");
const Path = require("fire-path");
const installer = require('./install');

const gConfig = {
    adcolonyVer: adColonyPluginVersion()
};

function loadConfig() {
    let filePath = Path.join(os.homedir(), '.adcolony', 'creator', 'config.json');
    let tempCfg = loadJsonFile(filePath);
    for (let key in tempCfg) {
        gConfig[key] = tempCfg[key]
    }

    filePath = Path.join(Editor.projectInfo.path, '.adcolony', 'creator', 'config.json');
    tempCfg = loadJsonFile(filePath);
    for (let key in tempCfg) {
        gConfig[key] = tempCfg[key]
    }

    filePath = Path.join(Editor.projectInfo.path, '.adcolony', 'creator', '.cocos_plugin_adcolony_config.json');
    tempCfg = loadJsonFile(filePath);
    gConfig.adcolonyCfg = tempCfg;
}

function loadJsonFile(filePath) {
    let j = null;
    try {
        j = JSON.parse(FS.readFileSync(filePath, 'utf8'));
    } catch (err) {
        // Editor.log(err);
        j = {};
    }

    return j;
}

function saveJsonFile(filePath, data) {
    const dir_path = Path.dirname(filePath);
    FS.ensureDir(dir_path, function() {
        FS.writeFileSync(filePath, data);
    });
}

function saveProjectCfg() {
    let filePath = Path.join(Editor.projectInfo.path, '.adcolony', 'creator', 'config.json');
    let cfg = {
        projectPath: gConfig.projectPath,
        adcolonyVer: gConfig.adcolonyVer
    }
    saveJsonFile(filePath, JSON.stringify(cfg));
}

function adColonyPluginVersion() {
    let v = null;
    try {
        const j = JSON.parse(FS.readFileSync(Path.join(__dirname, 'package.json'), 'utf8'));
        v = j.version;
    } catch (err) {
        // Editor.log(err);
        v = '0.0.0';
    }

    return v;
}

ipcMain.on('config', function(event) {
    event.sender.send('config', gConfig);
});

ipcMain.on('saveConfig', function(event, args) {
    for (let key in args) {
        gConfig[key] = args[key]
    }
    let cfg = {
        active: gConfig.active
    }
    let filePath = Path.join(os.homedir(), '.adcolony', 'creator', 'config.json');
    saveJsonFile(filePath, JSON.stringify(cfg));

    filePath = Path.join(Editor.projectInfo.path, '.adcolony', 'creator', '.cocos_plugin_adcolony_config.json');
    cfg = gConfig.adcolonyCfg;
    saveJsonFile(filePath, JSON.stringify(cfg));
});

module.exports = {

    load: function() {
        // globalShortcut.register('f12', () => {
        //     let win = BrowserWindow.getFocusedWindow();
        //     if (!win) return;
        //     win.webContents.toggleDevTools();
        // });
        loadConfig();
        Editor.log('AdColony Plugin load');
    },

    unload: function() {
        Editor.log('AdColony Plugin unload');
    },

    messages: {
        'configure': function() {
            const win = new BrowserWindow({
                width: 640,
                height: 480,
                minWidth: 480,
                minHeight: 320
            });

            win.loadURL(`file://${Path.join(__dirname, 'pages')}/configure.html`);
        },
        'editor:build-finished': function(event, target) {
            if (!gConfig.active) {
                return;
            }
            Editor.log('Import Cocos Creator AdColony Plugin');
            const projRoot = Path.normalize(target.dest);
            const packageRoot = Path.join(__dirname, 'plugin');
            gConfig.projectPath = projRoot;
            let success = false;
            try {
                gConfig.adcolonyVer = adColonyPluginVersion();
                installer.setInfo({version: gConfig.adcolonyVer});
                success = installer.installAdColony(projRoot, packageRoot, Editor.projectInfo.path);
            } catch(err) {
                Editor.log(err);
                success = false;
            }
            if (success) {
                saveProjectCfg();
                Editor.log('Import Cocos Creator AdColony success');
            } else {
                Editor.log('Import Cocos Creator AdColony failed');
            }
        },
        'editor:build-start': function(event, target) {
        },
        'updateConfig': function() {
        }
    },
};

