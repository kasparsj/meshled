import React, { useState, useEffect } from 'react';
import { X, Wifi, Eye, EyeOff } from 'lucide-react';
import { useToast } from '../contexts/ToastContext.jsx';

const WifiModal = ({ isOpen, onClose, onSave, currentSSID = '', currentPassword = '' }) => {
    const { showToast } = useToast();
    const [ssid, setSsid] = useState(currentSSID);
    const [password, setPassword] = useState(currentPassword);
    const [showPassword, setShowPassword] = useState(false);
    const [saving, setSaving] = useState(false);
    const [error, setError] = useState('');

    useEffect(() => {
        if (!isOpen) {
            return;
        }
        setSsid(currentSSID || '');
        setPassword(currentPassword || '');
        setError('');
    }, [isOpen, currentSSID, currentPassword]);

    // Handle ESC key to close modal
    useEffect(() => {
        const handleKeyDown = (event) => {
            if (event.key === 'Escape' && isOpen && !saving) {
                onClose();
            }
        };

        if (isOpen) {
            document.addEventListener('keydown', handleKeyDown);
            return () => document.removeEventListener('keydown', handleKeyDown);
        }
    }, [isOpen, onClose, saving]);

    if (!isOpen) return null;

    const handleSave = async () => {
        if (!ssid.trim()) {
            setError('SSID is required.');
            return;
        }

        setError('');
        setSaving(true);
        try {
            await onSave(ssid, password);
            showToast('WiFi settings saved. Device is restarting.', 'info', 4000);
            onClose();
        } catch (error) {
            console.error('Failed to save WiFi settings:', error);
            setError(error.message || 'Failed to save WiFi settings. Please try again.');
            showToast('Failed to save WiFi settings.', 'error');
        } finally {
            setSaving(false);
        }
    };

    const handleClose = () => {
        if (!saving) {
            onClose();
        }
    };

    return (
        <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
            <div className="bg-zinc-800 rounded-lg p-6 w-full max-w-md mx-4">
                <div className="flex items-center justify-between mb-4">
                    <h3 className="text-lg font-semibold flex items-center gap-2">
                        <Wifi size={20} />
                        WiFi Configuration
                    </h3>
                    <button
                        onClick={handleClose}
                        disabled={saving}
                        className="text-zinc-400 hover:text-white disabled:opacity-50"
                    >
                        <X size={20} />
                    </button>
                </div>

                <div className="space-y-4">
                    <div>
                        <label className="block text-sm text-zinc-300 mb-2">WiFi Network (SSID)</label>
                        <input
                            type="text"
                            value={ssid}
                            onChange={(e) => setSsid(e.target.value)}
                            className="w-full bg-zinc-700 border border-zinc-600 rounded px-3 py-2 text-white"
                            placeholder="Enter WiFi network name"
                            disabled={saving}
                        />
                    </div>

                    <div>
                        <label className="block text-sm text-zinc-300 mb-2">Password</label>
                        <div className="relative">
                            <input
                                type={showPassword ? 'text' : 'password'}
                                value={password}
                                onChange={(e) => setPassword(e.target.value)}
                                className="w-full bg-zinc-700 border border-zinc-600 rounded px-3 py-2 pr-10 text-white"
                                placeholder="Enter WiFi password"
                                disabled={saving}
                            />
                            <button
                                type="button"
                                onClick={() => setShowPassword(!showPassword)}
                                className="absolute right-2 top-1/2 transform -translate-y-1/2 text-zinc-400 hover:text-white"
                                disabled={saving}
                            >
                                {showPassword ? <EyeOff size={18} /> : <Eye size={18} />}
                            </button>
                        </div>
                    </div>

                    <div className="bg-yellow-900/20 border border-yellow-600/30 rounded p-3">
                        <p className="text-yellow-300 text-sm">
                            <strong>Warning:</strong> The device will restart after updating WiFi settings. 
                            Make sure you can connect to the new network to regain access.
                        </p>
                    </div>
                    {error && (
                        <div className="bg-red-900/30 border border-red-500/50 rounded p-3 text-sm text-red-200">
                            {error}
                        </div>
                    )}
                </div>

                <div className="flex justify-end gap-3 mt-6">
                    <button
                        onClick={handleClose}
                        disabled={saving}
                        className="px-4 py-2 text-zinc-400 hover:text-white disabled:opacity-50"
                    >
                        Cancel
                    </button>
                    <button
                        onClick={handleSave}
                        disabled={saving || !ssid.trim()}
                        className="bg-blue-600 hover:bg-blue-700 disabled:bg-blue-800 text-white px-6 py-2 rounded-lg"
                    >
                        {saving ? 'Updating...' : 'Update WiFi'}
                    </button>
                </div>
            </div>
        </div>
    );
};

export default WifiModal;
