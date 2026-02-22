import {Power, Wifi, Settings, Cpu, Radio} from "lucide-react";
import React, { useState, useEffect } from "react";
import useDeviceInfo from "../hooks/useDeviceInfo";
import useSettings from "../hooks/useSettings";
import WifiModal from "../components/WifiModal";

const SettingsTab = () => {
    const { deviceInfo, loading: deviceInfoLoading, error: deviceInfoError } = useDeviceInfo();
    const { 
        getSettings, 
        saveSettings: saveSettingsHook, 
        updateWifi,
        restartDevice, 
        loading: settingsLoading, 
        error: settingsError,
        constants: { LED_TYPES, COLOR_ORDERS, LED_LIBRARIES, OBJECT_TYPES }
    } = useSettings();
    
    // Settings state with proper enum values
    const [settings, setSettings] = useState({
        maxBrightness: 255,
        deviceHostname: '',
        pixelCount1: 0,
        pixelCount2: 0,
        pixelPin1: 0,
        pixelPin2: 0,
        pixelDensity: 60,
        ledType: 0, // LED_WS2812
        colorOrder: 4, // CO_GRB
        ledLibrary: 1, // LIB_FASTLED
        objectType: 0, // OBJ_HEPTAGON919
        oscEnabled: false,
        oscPort: 8000,
        otaEnabled: true,
        otaPort: 3232,
        otaPassword: '',
        savedSSID: '',
        savedPassword: ''
    });
    
    const [saveMessage, setSaveMessage] = useState('');
    const [showWifiModal, setShowWifiModal] = useState(false);

    const toNumberOrNull = (value) => {
        const num = Number(value);
        return Number.isFinite(num) ? num : null;
    };

    const formatNumber = (value, digits = 0, suffix = '') => {
        const num = toNumberOrNull(value);
        if (num === null) return 'N/A';
        return `${num.toFixed(digits)}${suffix}`;
    };

    // Load current settings from device
    useEffect(() => {
        const loadSettings = async () => {
            try {
                const data = await getSettings();
                setSettings(prevSettings => ({
                    ...prevSettings,
                    ...data
                }));
            } catch (error) {
                console.error('Failed to load settings:', error);
            }
        };
        
        loadSettings();
    }, [getSettings]);

    const handleWifiSave = async (ssid, password) => {
        await updateWifi(ssid, password);
        // Device will restart automatically after WiFi update
    };

    const handleRestartDevice = async () => {
        try {
            await restartDevice();
            console.log('Restart command sent');
        } catch (error) {
            console.error('Failed to restart device:', error);
        }
    };

    const handleSaveSettings = async () => {
        setSaveMessage('');
        
        try {
            await saveSettingsHook(settings);
            setSaveMessage('Settings saved successfully!');
            setTimeout(() => setSaveMessage(''), 3000);
        } catch (error) {
            console.error('Failed to save settings:', error);
            setSaveMessage('Error saving settings');
        }
    };

    const updateSetting = (key, value) => {
        setSettings(prev => ({
            ...prev,
            [key]: value
        }));
    };

    const powerWatts = toNumberOrNull(deviceInfo?.leds?.pwr);
    const activeLights = deviceInfo?.activeLights ?? deviceInfo?.leds?.count ?? 'N/A';
    const freeMemoryKB = toNumberOrNull(deviceInfo?.freeMemory);
    const storageUsedKB = toNumberOrNull(deviceInfo?.storageUsed);
    const storageTotalKB = toNumberOrNull(deviceInfo?.storageTotal);
    const storagePercent = (storageUsedKB !== null && storageTotalKB && storageTotalKB > 0)
        ? Math.max(0, Math.min(100, (storageUsedKB / storageTotalKB) * 100))
        : 0;

    return (
        <div className="space-y-6">
            <h2 className="text-2xl font-bold">Settings</h2>

            {(deviceInfoLoading || settingsLoading) && <div className="text-zinc-400 animate-pulse">Loading...</div>}
            {(deviceInfoError || settingsError) && <div className="text-red-500">{deviceInfoError || settingsError}</div>}

            {deviceInfo && (
                <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
                    <div className="bg-zinc-700 p-4 rounded-lg">
                        <h3 className="text-lg font-semibold mb-4">Device Info</h3>
                        <div className="space-y-2">
                            <p><span className="text-zinc-300">WiFi SSID:</span> <span
                                className="text-sky-400">{deviceInfo?.wifi?.ssid ?? 'N/A'}</span></p>
                            <p><span className="text-zinc-300">IP Address:</span> <span
                                className="text-sky-400">{deviceInfo?.ip ?? 'N/A'}</span></p>
                            <p><span className="text-zinc-300">Total Consumption:</span> <span
                                className="text-sky-400">{powerWatts !== null ? `${powerWatts.toFixed(2)} watts` : 'N/A'}</span></p>
                            <p><span className="text-zinc-300">Active Lights:</span> <span
                                className="text-sky-400">{activeLights}</span></p>
                            <p><span className="text-zinc-300">Free Memory:</span> <span
                                className="text-sky-400">{formatNumber(freeMemoryKB, 0, ' KB')}</span></p>
                            <p><span className="text-zinc-300">FPS:</span> <span
                                className="text-sky-400">{deviceInfo?.fps ?? 'N/A'}</span></p>
                        </div>
                    </div>

                    <div className="bg-zinc-700 p-4 rounded-lg">
                        <h3 className="text-lg font-semibold mb-4">Storage</h3>
                        <div className="space-y-2">
                            <p><span className="text-zinc-300">Used:</span> <span
                                className="text-sky-400">{formatNumber(storageUsedKB, 0, ' KB')}</span></p>
                            <p><span className="text-zinc-300">Total:</span> <span
                                className="text-sky-400">{formatNumber(storageTotalKB, 0, ' KB')}</span></p>
                            <div className="w-full bg-zinc-600 rounded-full h-2 mt-2">
                                <div
                                    className="bg-sky-600 h-2 rounded-full"
                                    style={{width: `${storagePercent}%`}}
                                />
                            </div>
                        </div>
                    </div>
                </div>
            )}

            {/* General Settings */}
            <div className="bg-zinc-700 p-4 rounded-lg">
                <h3 className="text-lg font-semibold mb-4 flex items-center gap-2">
                    <Settings size={20} />
                    General Settings
                </h3>
                <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                    <div>
                        <label className="block text-sm text-zinc-300 mb-2">Max Brightness</label>
                        <input
                            type="range"
                            min="1"
                            max="255"
                            value={settings.maxBrightness}
                            onChange={(e) => updateSetting('maxBrightness', parseInt(e.target.value))}
                            className="w-full"
                        />
                        <span className="text-sky-400 text-sm">{settings.maxBrightness}</span>
                    </div>
                    <div>
                        <label className="block text-sm text-zinc-300 mb-2">Device Hostname</label>
                        <input
                            type="text"
                            value={settings.deviceHostname}
                            onChange={(e) => updateSetting('deviceHostname', e.target.value)}
                            className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                            placeholder="Enter device hostname"
                        />
                    </div>
                </div>
            </div>

            {/* LED Configuration */}
            <div className="bg-zinc-700 p-4 rounded-lg">
                <h3 className="text-lg font-semibold mb-4 flex items-center gap-2">
                    <Cpu size={20} />
                    LED Configuration
                </h3>
                <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                    <div>
                        <label className="block text-sm text-zinc-300 mb-2">LED Type</label>
                        <select
                            value={settings.ledType}
                            onChange={(e) => updateSetting('ledType', parseInt(e.target.value))}
                            className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                        >
                            {Object.entries(LED_TYPES).map(([value, label]) => (
                                <option key={value} value={value}>{label}</option>
                            ))}
                        </select>
                    </div>
                    <div>
                        <label className="block text-sm text-zinc-300 mb-2">Pixel Density</label>
                        <select
                            value={settings.pixelDensity}
                            onChange={(e) => updateSetting('pixelDensity', parseInt(e.target.value))}
                            className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                        >
                            <option value="30">30 pixels/m</option>
                            <option value="60">60 pixels/m</option>
                            <option value="144">144 pixels/m</option>
                        </select>
                    </div>
                    <div>
                        <label className="block text-sm text-zinc-300 mb-2">Pixel Count 1</label>
                        <input
                            type="number"
                            min="1"
                            max="1000"
                            value={settings.pixelCount1}
                            onChange={(e) => updateSetting('pixelCount1', parseInt(e.target.value))}
                            className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                        />
                    </div>
                    <div>
                        <label className="block text-sm text-zinc-300 mb-2">Pixel Count 2</label>
                        <input
                            type="number"
                            min="0"
                            max="1000"
                            value={settings.pixelCount2}
                            onChange={(e) => updateSetting('pixelCount2', parseInt(e.target.value))}
                            className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                        />
                    </div>
                    <div>
                        <label className="block text-sm text-zinc-300 mb-2">Pixel Pin 1</label>
                        <input
                            type="number"
                            min="0"
                            max="33"
                            value={settings.pixelPin1}
                            onChange={(e) => updateSetting('pixelPin1', parseInt(e.target.value))}
                            className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                        />
                    </div>
                    <div>
                        <label className="block text-sm text-zinc-300 mb-2">Pixel Pin 2</label>
                        <input
                            type="number"
                            min="0"
                            max="33"
                            value={settings.pixelPin2}
                            onChange={(e) => updateSetting('pixelPin2', parseInt(e.target.value))}
                            className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                        />
                    </div>
                    <div>
                        <label className="block text-sm text-zinc-300 mb-2">Color Order</label>
                        <select
                            value={settings.colorOrder}
                            onChange={(e) => updateSetting('colorOrder', parseInt(e.target.value))}
                            className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                        >
                            {Object.entries(COLOR_ORDERS).map(([value, label]) => (
                                <option key={value} value={value}>{label}</option>
                            ))}
                        </select>
                    </div>
                    <div>
                        <label className="block text-sm text-zinc-300 mb-2">LED Library</label>
                        <select
                            value={settings.ledLibrary}
                            onChange={(e) => updateSetting('ledLibrary', parseInt(e.target.value))}
                            className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                        >
                            {Object.entries(LED_LIBRARIES).map(([value, label]) => (
                                <option key={value} value={value}>{label}</option>
                            ))}
                        </select>
                    </div>
                    <div>
                        <label className="block text-sm text-zinc-300 mb-2">Object Type</label>
                        <select
                            value={settings.objectType}
                            onChange={(e) => updateSetting('objectType', parseInt(e.target.value))}
                            className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                        >
                            {Object.entries(OBJECT_TYPES).map(([value, label]) => (
                                <option key={value} value={value}>{label}</option>
                            ))}
                        </select>
                    </div>
                </div>
            </div>

            {/* Communication Settings */}
            <div className="bg-zinc-700 p-4 rounded-lg">
                <h3 className="text-lg font-semibold mb-4 flex items-center gap-2">
                    <Radio size={20} />
                    Communication Settings
                </h3>
                <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                    <div>
                        <label className="block text-sm text-zinc-300 mb-2">OSC Enabled</label>
                        <label className="flex items-center">
                            <input
                                type="checkbox"
                                checked={settings.oscEnabled}
                                onChange={(e) => updateSetting('oscEnabled', e.target.checked)}
                                className="mr-2"
                            />
                            <span>Enable OSC communication</span>
                        </label>
                    </div>
                    {settings.oscEnabled && (
                        <div>
                            <label className="block text-sm text-zinc-300 mb-2">OSC Port</label>
                            <input
                                type="number"
                                min="1024"
                                max="65535"
                                value={settings.oscPort}
                                onChange={(e) => updateSetting('oscPort', parseInt(e.target.value))}
                                className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                            />
                        </div>
                    )}
                    <div>
                        <label className="block text-sm text-zinc-300 mb-2">OTA Updates Enabled</label>
                        <label className="flex items-center">
                            <input
                                type="checkbox"
                                checked={settings.otaEnabled}
                                onChange={(e) => updateSetting('otaEnabled', e.target.checked)}
                                className="mr-2"
                            />
                            <span>Enable over-the-air updates</span>
                        </label>
                    </div>
                    {settings.otaEnabled && (
                        <>
                            <div>
                                <label className="block text-sm text-zinc-300 mb-2">OTA Port</label>
                                <input
                                    type="number"
                                    min="1024"
                                    max="65535"
                                    value={settings.otaPort}
                                    onChange={(e) => updateSetting('otaPort', parseInt(e.target.value))}
                                    className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                                />
                            </div>
                            <div className="md:col-start-2">
                                <label className="block text-sm text-zinc-300 mb-2">OTA Password</label>
                                <input
                                    type="password"
                                    value={settings.otaPassword}
                                    onChange={(e) => updateSetting('otaPassword', e.target.value)}
                                    className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                                    placeholder="Enter OTA password"
                                />
                            </div>
                        </>
                    )}
                </div>
            </div>

            {/* Save Settings */}
            <div className="bg-zinc-700 p-4 rounded-lg">
                <div className="flex items-center justify-between">
                    <div>
                        <h3 className="text-lg font-semibold">Save Configuration</h3>
                        <p className="text-sm text-zinc-400">Changes will be applied immediately and saved to device memory.</p>
                    </div>
                    <button
                        onClick={handleSaveSettings}
                        disabled={settingsLoading}
                        className="bg-green-600 hover:bg-green-700 disabled:bg-green-800 px-6 py-3 rounded-lg flex items-center gap-2"
                    >
                        {settingsLoading ? 'Saving...' : 'Save Settings'}
                    </button>
                </div>
                {saveMessage && (
                    <div className={`mt-2 text-sm ${saveMessage.includes('success') ? 'text-green-400' : 'text-red-400'}`}>
                        {saveMessage}
                    </div>
                )}
            </div>

            <div className="bg-zinc-700 p-4 rounded-lg">
                <h3 className="text-lg font-semibold mb-4">Actions</h3>
                <div className="flex gap-4">
                    <button
                        onClick={() => setShowWifiModal(true)}
                        className="bg-yellow-600 hover:bg-yellow-700 px-4 py-2 rounded-lg flex items-center gap-2"
                    >
                        <Wifi size={18}/>
                        Configure WiFi
                    </button>
                    <button
                        onClick={handleRestartDevice}
                        className="bg-red-600 hover:bg-red-700 px-4 py-2 rounded-lg flex items-center gap-2"
                    >
                        <Power size={18}/>
                        Restart Device
                    </button>
                </div>
            </div>

            <WifiModal
                isOpen={showWifiModal}
                onClose={() => setShowWifiModal(false)}
                onSave={handleWifiSave}
                currentSSID={settings.savedSSID}
                currentPassword={settings.savedPassword}
            />
        </div>
    );
};

export default SettingsTab;
