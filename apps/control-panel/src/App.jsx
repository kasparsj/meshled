import React, { useState, useEffect } from 'react';
import { Routes, Route, useNavigate, useLocation } from 'react-router-dom';
import { Settings, Layers, Palette, Zap, Eye, Clock } from 'lucide-react';
import LayersTab from "./tabs/Layers.jsx";
import PalettesTab from "./tabs/Palettes.jsx";
import EmitterTab from "./tabs/Emitter.jsx";
import ModelTab from "./tabs/Model.jsx";
import SettingsTab from "./tabs/Settings.jsx";
import TimelineTab from "./tabs/Timeline.jsx";
import TabButton from "./components/TabButton.jsx";
import DeviceSelector from "./components/DeviceSelector.jsx";
import DeviceManagementModal from "./components/DeviceManagementModal.jsx";
import { DeviceProvider } from "./contexts/DeviceContext.jsx";
//import useDeviceInfo from "./hooks/useDeviceInfo.js";

const isIpv4Host = (value) => {
    const parts = String(value || '').split('.');
    if (parts.length !== 4) {
        return false;
    }
    return parts.every((part) => {
        if (!/^\d{1,3}$/.test(part)) {
            return false;
        }
        const num = Number(part);
        return num >= 0 && num <= 255;
    });
};

const isIpv6Host = (value) => {
    const host = String(value || '').trim();
    return host.includes(':') && /^[0-9a-f:]+$/i.test(host);
};

const detectDirectDeviceHost = () => {
    if (typeof window === 'undefined') {
        return '';
    }

    const hostname = String(window.location.hostname || '').trim().toLowerCase();
    if (!hostname || hostname === 'localhost' || hostname === '127.0.0.1' || hostname === '::1') {
        return '';
    }

    if (!isIpv4Host(hostname) && !isIpv6Host(hostname)) {
        return '';
    }

    return String(window.location.host || hostname).trim();
};

const LEDController = () => {
    const navigate = useNavigate();
    const location = useLocation();
    const directDeviceHost = detectDirectDeviceHost();
    const isDirectDeviceMode = Boolean(directDeviceHost);
    
    // Get current tab from URL path
    const getCurrentTab = () => {
        const path = location.pathname.substring(1) || 'layers'; // Remove leading slash, default to layers
        return ['layers', 'palettes', 'emitter', 'model', 'timeline', 'settings'].includes(path) ? path : 'layers';
    };
    
    const activeTab = getCurrentTab();
    const [devices, setDevices] = useState(() => (isDirectDeviceMode ? [directDeviceHost] : []));
    const [selectedDevice, setSelectedDevice] = useState(() => (isDirectDeviceMode ? directDeviceHost : ''));
    const [showDeviceModal, setShowDeviceModal] = useState(false);
    //const {deviceInfo, loading, error} = useDeviceInfo();

    // Load devices from localStorage on mount
    useEffect(() => {
        if (isDirectDeviceMode) {
            setDevices([directDeviceHost]);
            setSelectedDevice(directDeviceHost);
            return;
        }

        const savedDevices = localStorage.getItem('ledController_devices');
        const savedSelectedDevice = localStorage.getItem('ledController_selectedDevice');
        
        if (savedDevices) {
            const parsedDevices = JSON.parse(savedDevices);
            setDevices(parsedDevices);
            
            if (savedSelectedDevice && parsedDevices.includes(savedSelectedDevice)) {
                setSelectedDevice(savedSelectedDevice);
            } else if (parsedDevices.length > 0) {
                setSelectedDevice(parsedDevices[0]);
            }
        }
    }, [isDirectDeviceMode, directDeviceHost]);

    // Save devices to localStorage when devices change
    useEffect(() => {
        if (isDirectDeviceMode) {
            return;
        }

        if (devices.length > 0) {
            localStorage.setItem('ledController_devices', JSON.stringify(devices));
        } else {
            localStorage.removeItem('ledController_devices');
        }
    }, [devices, isDirectDeviceMode]);

    // Save selected device to localStorage when it changes
    useEffect(() => {
        if (isDirectDeviceMode) {
            return;
        }

        if (selectedDevice) {
            localStorage.setItem('ledController_selectedDevice', selectedDevice);
        } else {
            localStorage.removeItem('ledController_selectedDevice');
        }
    }, [selectedDevice, isDirectDeviceMode]);

    const handleDevicesChange = (newDevices) => {
        if (isDirectDeviceMode) {
            return;
        }

        setDevices(newDevices);
        // If current selected device is removed, select first available or clear
        if (selectedDevice && !newDevices.includes(selectedDevice)) {
            setSelectedDevice(newDevices.length > 0 ? newDevices[0] : '');
            return;
        }

        // Auto-select the first device when none is currently selected.
        if (!selectedDevice && newDevices.length > 0) {
            setSelectedDevice(newDevices[0]);
        }
    };

    // Redirect to /layers if on root path
    useEffect(() => {
        if (location.pathname === '/') {
            navigate('/layers', { replace: true });
        }
    }, [location.pathname, navigate]);

    return (
        <div className="min-h-screen bg-zinc-900 text-white">
            <div className="container mx-auto px-4 py-6 max-w-6xl">
                {/* Header */}
                <div className="mb-8 grid grid-cols-4 gap-6">
                    <div className="col-span-3 flex flex-col">
                        <h1 className="text-3xl font-bold mb-2 flex-1">LED Controller</h1>
                        <div className="flex flex-wrap gap-2">
                            <TabButton id="layers" icon={Layers} label="Layers" active={activeTab === 'layers'} onClick={(tab) => navigate(`/${tab}`)} />
                            <TabButton id="palettes" icon={Palette} label="Palettes" active={activeTab === 'palettes'} onClick={(tab) => navigate(`/${tab}`)} />
                            <TabButton id="emitter" icon={Zap} label="Emitter" active={activeTab === 'emitter'} onClick={(tab) => navigate(`/${tab}`)} />
                            <TabButton id="model" icon={Eye} label="Model" active={activeTab === 'model'} onClick={(tab) => navigate(`/${tab}`)} />
                            <TabButton id="timeline" icon={Clock} label="Timeline" active={activeTab === 'timeline'} onClick={(tab) => navigate(`/${tab}`)} />
                            <TabButton id="settings" icon={Settings} label="Settings" active={activeTab === 'settings'} onClick={(tab) => navigate(`/${tab}`)} />
                        </div>
                    </div>
                    <div className="col-span-1">
                        {isDirectDeviceMode ? (
                            <div className="bg-zinc-800 rounded-lg p-4">
                                <div className="text-xs uppercase tracking-wide text-zinc-400 mb-2">Connected Device</div>
                                <div className="font-mono text-sky-400 break-all">{selectedDevice || directDeviceHost}</div>
                            </div>
                        ) : (
                            <>
                                <DeviceSelector
                                    devices={devices}
                                    selectedDevice={selectedDevice}
                                    onDeviceSelect={setSelectedDevice}
                                    onManageDevices={() => setShowDeviceModal(true)}
                                />
                                <DeviceManagementModal
                                    isOpen={showDeviceModal}
                                    onClose={() => setShowDeviceModal(false)}
                                    devices={devices}
                                    setDevices={handleDevicesChange}
                                />
                            </>
                        )}
                    </div>
                </div>

                {/* Main Content */}
                <div className="bg-zinc-800 rounded-lg p-6">
                    <DeviceProvider selectedDevice={selectedDevice}>
                        <Routes>
                            <Route path="/layers" element={<LayersTab />} />
                            <Route path="/palettes" element={<PalettesTab />} />
                            <Route path="/emitter" element={<EmitterTab />} />
                            <Route path="/model" element={<ModelTab devices={devices} />} />
                            <Route path="/timeline" element={<TimelineTab />} />
                            <Route path="/settings" element={<SettingsTab />} />
                            <Route path="*" element={<LayersTab />} />
                        </Routes>
                    </DeviceProvider>
                </div>
            </div>
        </div>
    );
};

export default LEDController;
