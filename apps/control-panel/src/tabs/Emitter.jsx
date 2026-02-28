import { Zap } from 'lucide-react';
import React from 'react';
import useEmitter from '../hooks/useEmitter.js';
import { useToast } from '../contexts/ToastContext.jsx';

const EmitterTab = () => {
    const { showToast } = useToast();
    const {
        settings,
        saving,
        error,
        updateMinSpeed,
        updateMaxSpeed,
        updateMinDuration,
        updateMaxDuration,
        toggleAuto,
    } = useEmitter();

    const runMutation = async (operation, fallbackMessage) => {
        try {
            await operation();
        } catch (mutationError) {
            showToast(mutationError.message || fallbackMessage, 'error');
        }
    };

    return (
        <div className="space-y-6">
            <h2 className="text-2xl font-bold">Emitter Settings</h2>

            <div className="bg-zinc-700 p-4 rounded-lg">
                <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
                    <div>
                        <h3 className="text-lg font-semibold mb-4">Speed Range</h3>
                        <div className="space-y-4">
                            <div>
                                <label className="block text-sm text-zinc-300 mb-2">
                                    Min Speed ({settings.minSpeed.toFixed(1)})
                                </label>
                                <input
                                    type="range"
                                    min="0.1"
                                    max="5"
                                    step="0.1"
                                    value={settings.minSpeed}
                                    onChange={(event) => runMutation(
                                        () => updateMinSpeed(Number.parseFloat(event.target.value)),
                                        'Failed to update min speed',
                                    )}
                                    className="w-full"
                                    disabled={saving}
                                />
                            </div>
                            <div>
                                <label className="block text-sm text-zinc-300 mb-2">
                                    Max Speed ({settings.maxSpeed.toFixed(1)})
                                </label>
                                <input
                                    type="range"
                                    min="1"
                                    max="20"
                                    step="0.5"
                                    value={settings.maxSpeed}
                                    onChange={(event) => runMutation(
                                        () => updateMaxSpeed(Number.parseFloat(event.target.value)),
                                        'Failed to update max speed',
                                    )}
                                    className="w-full"
                                    disabled={saving}
                                />
                            </div>
                        </div>
                    </div>

                    <div>
                        <h3 className="text-lg font-semibold mb-4">Duration Range</h3>
                        <div className="space-y-4">
                            <div>
                                <label className="block text-sm text-zinc-300 mb-2">
                                    Min Duration (ms)
                                </label>
                                <input
                                    type="number"
                                    min="100"
                                    max="5000"
                                    step="100"
                                    value={settings.minDuration}
                                    onChange={(event) => runMutation(
                                        () => updateMinDuration(Number.parseInt(event.target.value, 10)),
                                        'Failed to update min duration',
                                    )}
                                    className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                                    disabled={saving}
                                />
                            </div>
                            <div>
                                <label className="block text-sm text-zinc-300 mb-2">
                                    Max Duration (ms)
                                </label>
                                <input
                                    type="number"
                                    min="1000"
                                    max="50000"
                                    step="1000"
                                    value={settings.maxDuration}
                                    onChange={(event) => runMutation(
                                        () => updateMaxDuration(Number.parseInt(event.target.value, 10)),
                                        'Failed to update max duration',
                                    )}
                                    className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                                    disabled={saving}
                                />
                            </div>
                        </div>
                    </div>
                </div>

                <div className="mt-6 flex items-center gap-4">
                    <button
                        className={`px-4 py-2 rounded-lg flex items-center gap-2 ${
                            settings.autoEnabled ? 'bg-orange-500 hover:bg-orange-400' : 'bg-orange-600 hover:bg-orange-700'
                        }`}
                        onClick={() => runMutation(toggleAuto, 'Failed to toggle auto mode')}
                        disabled={saving}
                    >
                        <Zap size={18} />
                        {settings.autoEnabled ? 'Disable Auto Mode' : 'Enable Auto Mode'}
                    </button>
                    {saving && <span className="text-zinc-400 text-sm">Saving...</span>}
                </div>

                {error && (
                    <p className="mt-4 text-sm text-red-300">{error}</p>
                )}

                <p className="mt-3 text-xs text-zinc-400">
                    Current values are initialized from firmware defaults in this UI and updated live as you change controls.
                </p>
            </div>
        </div>
    );
};

export default EmitterTab;
