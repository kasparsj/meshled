import React, { Suspense, lazy, useEffect, useMemo, useState } from 'react';
import { Routes, Route, useNavigate, useLocation } from 'react-router-dom';
import { Settings, Layers, Palette, Zap, Eye, KeyRound } from 'lucide-react';
import TabButton from './components/TabButton.jsx';
import DeviceSelector from './components/DeviceSelector.jsx';
import DeviceManagementModal from './components/DeviceManagementModal.jsx';
import ApiTokenModal from './components/ApiTokenModal.jsx';
import { DeviceProvider, useDevice } from './contexts/DeviceContext.jsx';

const LayersTab = lazy(() => import('./tabs/Layers.jsx'));
const PalettesTab = lazy(() => import('./tabs/Palettes.jsx'));
const EmitterTab = lazy(() => import('./tabs/Emitter.jsx'));
const ModelTab = lazy(() => import('./tabs/Model.jsx'));
const SettingsTab = lazy(() => import('./tabs/Settings.jsx'));

const TIMELINE_ENABLED = false;
const TimelineTab = TIMELINE_ENABLED ? lazy(() => import('./tabs/Timeline.jsx')) : null;

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

const sanitizeDeviceList = (raw) => {
    if (!Array.isArray(raw)) {
        return [];
    }

    const seen = new Set();
    const sanitized = [];
    for (const entry of raw) {
        const value = String(entry || '').trim();
        if (!value || seen.has(value)) {
            continue;
        }
        seen.add(value);
        sanitized.push(value);
    }

    return sanitized;
};

const ControllerContent = ({
    activeTab,
    devices,
    selectedDevice,
    setSelectedDevice,
    isDirectDeviceMode,
    directDeviceHost,
    showDeviceModal,
    setShowDeviceModal,
    handleDevicesChange,
    navigate,
}) => {
    const {
        authRequired,
        authError,
        hasApiToken,
        apiToken,
        setApiToken,
        clearApiToken,
    } = useDevice();

    const [showApiTokenModal, setShowApiTokenModal] = useState(false);
    const forceApiTokenModal = authRequired && !hasApiToken;
    const timelineEnabled = TIMELINE_ENABLED;

    return (
        <>
            <div className="min-h-screen bg-zinc-900 text-white">
                <div className="container mx-auto px-4 py-6 max-w-6xl">
                    <div className="mb-8 grid grid-cols-1 lg:grid-cols-4 gap-6">
                        <div className="lg:col-span-3 flex flex-col">
                            <h1 className="text-3xl font-bold mb-2 flex-1">MeshLED</h1>
                            <div className="flex flex-wrap gap-2">
                                <TabButton id="layers" icon={Layers} label="Layers" active={activeTab === 'layers'} onClick={(tab) => navigate(`/${tab}`)} />
                                <TabButton id="palettes" icon={Palette} label="Palettes" active={activeTab === 'palettes'} onClick={(tab) => navigate(`/${tab}`)} />
                                <TabButton id="emitter" icon={Zap} label="Emitter" active={activeTab === 'emitter'} onClick={(tab) => navigate(`/${tab}`)} />
                                <TabButton id="model" icon={Eye} label="Model" active={activeTab === 'model'} onClick={(tab) => navigate(`/${tab}`)} />
                                {timelineEnabled && (
                                    <TabButton id="timeline" icon={Zap} label="Timeline" active={activeTab === 'timeline'} onClick={(tab) => navigate(`/${tab}`)} />
                                )}
                                <TabButton id="settings" icon={Settings} label="Settings" active={activeTab === 'settings'} onClick={(tab) => navigate(`/${tab}`)} />
                            </div>
                        </div>
                        <div className="lg:col-span-1 space-y-3">
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

                            <div className="bg-zinc-800 rounded-lg p-3 border border-zinc-700">
                                <button
                                    onClick={() => setShowApiTokenModal(true)}
                                    className="w-full bg-zinc-700 hover:bg-zinc-600 text-sm px-3 py-2 rounded flex items-center justify-center gap-2"
                                >
                                    <KeyRound size={14} />
                                    {hasApiToken ? 'Update API Token' : 'Set API Token'}
                                </button>
                                <p className="mt-2 text-xs text-zinc-400">
                                    {hasApiToken ? 'Token is configured for protected routes.' : 'No token configured.'}
                                </p>
                                {authError && (
                                    <p className="mt-2 text-xs text-red-300">{authError}</p>
                                )}
                            </div>
                        </div>
                    </div>

                    <div className="bg-zinc-800 rounded-lg p-6">
                        <Suspense fallback={<div className="text-zinc-400 animate-pulse">Loading tab...</div>}>
                            <Routes>
                                <Route path="/layers" element={<LayersTab />} />
                                <Route path="/palettes" element={<PalettesTab />} />
                                <Route path="/emitter" element={<EmitterTab />} />
                                <Route path="/model" element={<ModelTab devices={devices} />} />
                                {timelineEnabled && TimelineTab && <Route path="/timeline" element={<TimelineTab />} />}
                                <Route path="/settings" element={<SettingsTab />} />
                                <Route path="*" element={<LayersTab />} />
                            </Routes>
                        </Suspense>
                    </div>
                </div>
            </div>

            <ApiTokenModal
                isOpen={showApiTokenModal || forceApiTokenModal}
                onClose={() => setShowApiTokenModal(false)}
                onSave={setApiToken}
                onClear={clearApiToken}
                currentToken={apiToken}
                authError={authError}
                requireToken={forceApiTokenModal}
            />
        </>
    );
};

const LEDController = () => {
    const navigate = useNavigate();
    const location = useLocation();
    const directDeviceHost = detectDirectDeviceHost();
    const isDirectDeviceMode = Boolean(directDeviceHost);

    const validTabs = useMemo(() => {
        const tabs = ['layers', 'palettes', 'emitter', 'model', 'settings'];
        if (TIMELINE_ENABLED) {
            tabs.push('timeline');
        }
        return tabs;
    }, []);

    const activeTab = (() => {
        const path = location.pathname.substring(1) || 'layers';
        return validTabs.includes(path) ? path : 'layers';
    })();

    const [devices, setDevices] = useState(() => (isDirectDeviceMode ? [directDeviceHost] : []));
    const [selectedDevice, setSelectedDevice] = useState(() => (isDirectDeviceMode ? directDeviceHost : ''));
    const [showDeviceModal, setShowDeviceModal] = useState(false);
    const [storageHydrated, setStorageHydrated] = useState(false);

    useEffect(() => {
        if (isDirectDeviceMode) {
            setDevices([directDeviceHost]);
            setSelectedDevice(directDeviceHost);
            setStorageHydrated(true);
            return;
        }

        const savedDevices = localStorage.getItem('ledController_devices');
        const savedSelectedDevice = localStorage.getItem('ledController_selectedDevice');

        let parsedDevices = [];
        if (savedDevices) {
            try {
                parsedDevices = sanitizeDeviceList(JSON.parse(savedDevices));
            } catch {
                localStorage.removeItem('ledController_devices');
                localStorage.removeItem('ledController_selectedDevice');
            }
        }

        setDevices(parsedDevices);

        if (savedSelectedDevice && parsedDevices.includes(savedSelectedDevice)) {
            setSelectedDevice(savedSelectedDevice);
        } else if (parsedDevices.length > 0) {
            setSelectedDevice(parsedDevices[0]);
        } else {
            setSelectedDevice('');
        }
        setStorageHydrated(true);
    }, [isDirectDeviceMode, directDeviceHost]);

    useEffect(() => {
        if (isDirectDeviceMode || !storageHydrated) {
            return;
        }

        if (devices.length > 0) {
            localStorage.setItem('ledController_devices', JSON.stringify(devices));
        } else {
            localStorage.removeItem('ledController_devices');
        }
    }, [devices, isDirectDeviceMode, storageHydrated]);

    useEffect(() => {
        if (isDirectDeviceMode || !storageHydrated) {
            return;
        }

        if (selectedDevice) {
            localStorage.setItem('ledController_selectedDevice', selectedDevice);
        } else {
            localStorage.removeItem('ledController_selectedDevice');
        }
    }, [selectedDevice, isDirectDeviceMode, storageHydrated]);

    const handleDevicesChange = (newDevices) => {
        if (isDirectDeviceMode) {
            return;
        }

        const sanitized = sanitizeDeviceList(newDevices);
        setDevices(sanitized);

        if (selectedDevice && !sanitized.includes(selectedDevice)) {
            setSelectedDevice(sanitized.length > 0 ? sanitized[0] : '');
            return;
        }

        if (!selectedDevice && sanitized.length > 0) {
            setSelectedDevice(sanitized[0]);
        }
    };

    useEffect(() => {
        if (location.pathname === '/') {
            navigate('/layers', { replace: true });
            return;
        }

        if (!TIMELINE_ENABLED && location.pathname === '/timeline') {
            navigate('/layers', { replace: true });
        }
    }, [location.pathname, navigate]);

    return (
        <DeviceProvider selectedDevice={selectedDevice}>
            <ControllerContent
                activeTab={activeTab}
                devices={devices}
                selectedDevice={selectedDevice}
                setSelectedDevice={setSelectedDevice}
                isDirectDeviceMode={isDirectDeviceMode}
                directDeviceHost={directDeviceHost}
                showDeviceModal={showDeviceModal}
                setShowDeviceModal={setShowDeviceModal}
                handleDevicesChange={handleDevicesChange}
                navigate={navigate}
            />
        </DeviceProvider>
    );
};

export default LEDController;
