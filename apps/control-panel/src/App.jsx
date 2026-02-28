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

const LEDController = () => {
    const navigate = useNavigate();
    const location = useLocation();
    
    // Get current tab from URL path
    const getCurrentTab = () => {
        const path = location.pathname.substring(1) || 'layers'; // Remove leading slash, default to layers
        return ['layers', 'palettes', 'emitter', 'model', 'timeline', 'settings'].includes(path) ? path : 'layers';
    };
    
    const activeTab = getCurrentTab();
    const [devices, setDevices] = useState([]);
    const [selectedDevice, setSelectedDevice] = useState('');
    const [showDeviceModal, setShowDeviceModal] = useState(false);
    //const {deviceInfo, loading, error} = useDeviceInfo();

    // Load devices from localStorage on mount
    useEffect(() => {
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
    }, []);

    // Save devices to localStorage when devices change
    useEffect(() => {
        if (devices.length > 0) {
            localStorage.setItem('ledController_devices', JSON.stringify(devices));
        } else {
            localStorage.removeItem('ledController_devices');
        }
    }, [devices]);

    // Save selected device to localStorage when it changes
    useEffect(() => {
        if (selectedDevice) {
            localStorage.setItem('ledController_selectedDevice', selectedDevice);
        } else {
            localStorage.removeItem('ledController_selectedDevice');
        }
    }, [selectedDevice]);

    const handleDevicesChange = (newDevices) => {
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
